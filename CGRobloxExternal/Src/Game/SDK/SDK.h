#pragma once
#include "../Offsets/Offsets.h"
#include "../../Memory/Communication.h"
#include <string>
#include <vector>
#include <cmath>

namespace RBX {

    struct ChildEntry {
        uintptr_t addr = 0;
        std::string name;
        std::string className;
    };

    struct Vec2 {
        float X{ 0 };
        float Y{ 0 };
    };

    struct Vec3 {
        float X{ 0 };
        float Y{ 0 };

        float Z{ 0 };
    };

    struct Vec4 {
        float X{ 0 };
        float Y{ 0 };
        float Z{ 0 };
        float W{ 0 };
    };


    struct Mat4 {
        float data[16];
    };

    struct CFrame {
        float data[12];

        Vec3 GetRightVector() const {
            return { data[0], data[3], data[6] };
        }

        Vec3 GetUpVector() const {
            return { data[1], data[4], data[7] };
        }

        Vec3 GetLookVector() const {
            return { data[2], data[5], data[8] };
        }

        Vec3 GetPosition() const {
            return { data[9], data[10], data[11] };
        }
    };

    class RbxInstance {
    public:
        uintptr_t Addr;

        RbxInstance(uintptr_t addr) : Addr(addr) {}

        std::string GetName() {
            uintptr_t namePtr = Coms->ReadMemory<uintptr_t>(Addr + offsets::Name);
            if (namePtr == 0) return "";
            
            return Coms->ReadGameString(namePtr);
        }

        std::string GetClass() {
            uintptr_t classDesc = Coms->ReadMemory<uintptr_t>(Addr + offsets::ClassDescriptor);
            if (classDesc == 0) return "";

            
            uintptr_t namePtr = Coms->ReadMemory<uintptr_t>(classDesc + offsets::ClassDescriptorToClassName);
            if (namePtr == 0) return "";
            
            return Coms->ReadGameString(namePtr);
        }

        RbxInstance GetParent() {
            return RbxInstance(Coms->ReadMemory<uintptr_t>(Addr + offsets::Parent));
        }


        std::vector<RbxInstance> GetChildList() {
            std::vector<RbxInstance> childList;
            if (Addr == 0) return childList;

            uintptr_t childStart = Coms->ReadMemory<uintptr_t>(Addr + offsets::Children);
            if (childStart < 0x10000 || childStart >= 0x7FFFFFFFFFFF) return childList;

            uintptr_t childEnd = Coms->ReadMemory<uintptr_t>(childStart + offsets::ChildrenEnd);
            uintptr_t ptr = Coms->ReadMemory<uintptr_t>(childStart);
            if (ptr == 0 || childEnd == 0 || ptr >= childEnd) return childList;

            childList.reserve(32);
            constexpr size_t kMaxChildren = 512;
            for (size_t i = 0; ptr != childEnd && i < kMaxChildren; ptr += 0x10, ++i) {
                uintptr_t childAddr = Coms->ReadMemory<uintptr_t>(ptr);
                if (childAddr != 0) childList.emplace_back(childAddr);
            }

            return childList;
        }

        std::vector<ChildEntry> GetChildEntries() {
            std::vector<ChildEntry> entries;
            auto children = GetChildList();
            entries.reserve(children.size());
            for (auto& child : children) {
                ChildEntry e;
                e.addr = child.Addr;
                e.name = child.GetName();
                e.className = child.GetClass();
                entries.push_back(std::move(e));
            }
            return entries;
        }

        RbxInstance FindChild(const std::string& targetName) {
            auto children = GetChildList();

            
            for (auto& child : children) {
                if (child.GetName() == targetName) {
                    return child;


                }
            }
            
            return RbxInstance(0);

        }

        RbxInstance FindChildByClass(const std::string& targetClass) {
            auto children = GetChildList();
            
            for (auto& child : children) {

                if (child.GetClass() == targetClass) {
                    return child;
                }
            }
            
            return RbxInstance(0);
        }



        RbxInstance WaitChild(const std::string& targetName) {
            while (true) {
                auto children = GetChildList();
                

                for (auto& child : children) {
                    if (child.GetName() == targetName) {
                        return child;
                    }
                }
            }
        }

        uintptr_t GetPrimitivePtr() {
            return Coms->ReadMemory<uintptr_t>(Addr + offsets::Primitive);
        }


        Vec3 GetPos() {
            uintptr_t prim = GetPrimitivePtr();
            if (prim == 0) return {};
            return Coms->ReadMemory<Vec3>(prim + offsets::Position);
        }

        Vec3 GetSize() {
            uintptr_t prim = GetPrimitivePtr();
            if (prim == 0) return {};
            return Coms->ReadMemory<Vec3>(prim + offsets::PartSize);
        }

        CFrame GetCFrame() {
            uintptr_t prim = GetPrimitivePtr();
            return Coms->ReadMemory<CFrame>(prim + offsets::CFrame);
        }

        RbxInstance GetModelRef() {
            return RbxInstance(Coms->ReadMemory<uintptr_t>(Addr + offsets::ModelInstance));
        }


        float CalcDistance(const Vec3& targetPos) {
            Vec3 currentPos = GetPos();

            
            float dx = currentPos.X - targetPos.X;
            float dy = currentPos.Y - targetPos.Y;
            float dz = currentPos.Z - targetPos.Z;
            
            return sqrtf(dx * dx + dy * dy + dz * dz);
        }

        Vec3 GetCameraPos() {
            return Coms->ReadMemory<Vec3>(Addr + offsets::CameraPos);
        }
    };


    class RenderEngine : public RbxInstance {
    public:
        RenderEngine(uintptr_t addr) : RbxInstance(addr) {}

        Mat4 GetViewMat() {
            return Coms->ReadMemory<Mat4>(Addr + offsets::viewmatrix);
        }


        Vec2 WorldToViewport(const Vec3& worldPos) {
            Vec4 quat;
            
            Vec2 screenDims{ static_cast<float>(GetSystemMetrics(SM_CXSCREEN)), 
                            static_cast<float>(GetSystemMetrics(SM_CYSCREEN)) };

            Mat4 viewMat = GetViewMat();
            
            quat.X = (worldPos.X * viewMat.data[0]) + (worldPos.Y * viewMat.data[1]) + (worldPos.Z * viewMat.data[2]) + viewMat.data[3];
            quat.Y = (worldPos.X * viewMat.data[4]) + (worldPos.Y * viewMat.data[5]) + (worldPos.Z * viewMat.data[6]) + viewMat.data[7];

            quat.Z = (worldPos.X * viewMat.data[8]) + (worldPos.Y * viewMat.data[9]) + (worldPos.Z * viewMat.data[10]) + viewMat.data[11];
            quat.W = (worldPos.X * viewMat.data[12]) + (worldPos.Y * viewMat.data[13]) + (worldPos.Z * viewMat.data[14]) + viewMat.data[15];
            
            Vec2 screenPos;
            
            if (quat.W < 0.1f) {

                return screenPos;
            }

            
            Vec3 ndc;
            ndc.X = quat.X / quat.W;
            ndc.Y = quat.Y / quat.W;
            ndc.Z = quat.Z / quat.W;
            
            screenPos.X = (screenDims.X / 2.0f * ndc.X) + (screenDims.X / 2.0f);
            screenPos.Y = -(screenDims.Y / 2.0f * ndc.Y) + (screenDims.Y / 2.0f);

            
            return screenPos;
        }
    };

    inline void ModifyWalkSpeed(RbxInstance humanoid, float newSpeed) {
        Coms->WriteMemory(humanoid.Addr + offsets::WalkSpeed, newSpeed);
        Coms->WriteMemory(humanoid.Addr + offsets::WalkSpeedCheck, newSpeed);

    }



    inline void ModifyJumpPower(RbxInstance humanoid, float newPower) {
        Coms->WriteMemory(humanoid.Addr + offsets::JumpPower, newPower);
        Coms->WriteMemory(humanoid.Addr + offsets::JumpHeight, newPower);
    }

    inline uintptr_t FindChildAddr(const std::vector<ChildEntry>& entries, const char* targetName) {
        for (const auto& e : entries)
            if (e.name == targetName) return e.addr;
        return 0;
    }

    inline uintptr_t FindClassAddr(const std::vector<ChildEntry>& entries, const char* targetClass) {
        for (const auto& e : entries)
            if (e.className == targetClass) return e.addr;
        return 0;
    }

}

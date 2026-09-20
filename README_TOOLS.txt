CHEAT GLOBAL - BUILD & OFFSET TOOLS
===================================

REQUIREMENTS
------------
- Windows 10/11
- Visual Studio 2022 or Build Tools 2022
- "Desktop development with C++" workload
- Internet connection for offset updates


1) UPDATE_OFFSETS.bat
---------------------
- Detects the installed/running Roblox version.
- Downloads the archive for that exact version from offsets.imtheo.lol.
- Falls back to the latest dump only when the version archive is unavailable.
- Refuses to change files if the downloaded version does not match Roblox.
- Validates critical pointers and at least 25 project constants.
- Creates Offsets.h.backup before writing.
- Updates the version shown in the menu.

If Roblox has just updated and the service is still behind, wait until the
offset service publishes the same version. Do not force mismatched offsets.


2) BUILD_RELEASE.bat
--------------------
- Finds MSBuild automatically.
- Builds Release x64.
- Output:
  CGRobloxExternal\x64\Release\CGRobloxExternal.exe


3) UPDATE_AND_BUILD.bat
-----------------------
- Runs the safe offset updater.
- Builds only when the update succeeds.


RECOMMENDED UPDATE FLOW
-----------------------
1. Start Roblox and join a game.
2. Close the old external executable.
3. Run UPDATE_AND_BUILD.bat.
4. Start the newly built executable.


TROUBLESHOOTING
---------------
"VERSION MISMATCH"
The exact version archive was unavailable and the latest service build did
not match Roblox. No project files were changed. Wait for the archive.

"MSBuild was not found"
Install Visual Studio 2022 / Build Tools and select Desktop development
with C++.

"Attach failed: DataModel pointer"
Confirm that the version in Offsets.h matches the installed Roblox version,
then run UPDATE_OFFSETS.bat and rebuild.

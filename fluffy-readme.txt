- I followed this guide for setting up the source code: https://github.com/diasurgical/devilutionX/tree/4e8843ee7ca23bb30cb4920378c32bfa5f269c21
- These are the steps I did:
-- Install dependencies via vcpkg
-- Run "vcpkg integrate install" so Visual Studio can access to everything vcpkg installed
-- Set up project by selecting cmakelists.txt in Visual Studio

- I followed this guide for defining launch arguments for debugging: https://stackoverflow.com/questions/41864259/how-to-set-working-directory-for-visual-studio-2017-rc-cmake-project/42973332#42973332
-- The guide says to select Debug and Launch Settings after "right clicking the relevant CMakeLists.txt" but the option is actually via the Debug drop-down menu
-- I added this line to launch.vs.json to skip intro videos and to define data path: "args": [ "-n", "--data-dir \"D:\\spill\\GOG Galaxy\\Games\\Diablo\"" ]
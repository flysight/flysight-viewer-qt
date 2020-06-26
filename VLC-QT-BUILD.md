#BUILDING VLC-Qt

First, clone the VLC-Qt repository:

```
git clone https://github.com/vlc-qt/vlc-qt
cd vlc-qt
git submodule init
git submodule update
```

Now we want to build it using VS 2019 x64. Here are the instructions:

https://github.com/vlc-qt/vlc-qt/blob/master/BUILDING.md

The instructions seem to indicate we can build using VS 2019 and nmake. Visual Studio Express does not include x64 tools, so instead we download the VS 2019 Build Tools:

https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=BuildTools

Next we install Qt 5.15 for VS 2019 x64.

Then we download the VLC 3.0.11 SDK here:

https://download.videolan.org/pub/videolan/vlc/3.0.11/win64/

The SDK files are now located in "C:\Users\Michael\Downloads\vlc-3.0.11-win64\vlc-3.0.11\sdk".

Now we run the "x64 Native Tools Command Prompt for VS 2019" and change to the VLC-Qt folder. Then we run the following commands:

```
md build
cd build
cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Debug ^
  -DCMAKE_INSTALL_PREFIX="C:\Users\Michael\Desktop\vlc-qt\install\msvc64" ^
  -DLIBVLC_LIBRARY="C:\Users\Michael\Downloads\vlc-3.0.11-win64\vlc-3.0.11\sdk\lib\libvlc.lib" ^
  -DLIBVLCCORE_LIBRARY="C:\Users\Michael\Downloads\vlc-3.0.11-win64\vlc-3.0.11\sdk\lib\libvlccore.lib" ^
  -DLIBVLC_INCLUDE_DIR="C:\Users\Michael\Downloads\vlc-3.0.11-win64\vlc-3.0.11\sdk\include" ^
  -DCMAKE_PREFIX_PATH="C:\Qt\5.15.0\msvc2019_64\lib\cmake"
nmake
nmake install
```

We now have the debug version of VLC-Qt built and installed in "C:\Users\Michael\Desktop\vlc-qt\install\msvc64".

Finally, we modify the cmake command above to build the release version, then repeat the two nmake commands.

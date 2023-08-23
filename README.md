# FlySight Viewer

## Libraries

1. FlySight Viewer should be built using Qt 5.5.1. Later versions of Qt do not include QWebView, which is used by MapView.
2. VLC-Qt builds can be found here: https://github.com/vlc-qt/vlc-qt/releases. For Qt 5.5, use VLC-Qt 1.0.
3. wwWidgets is located here: http://www.wysota.eu.org/wwwidgets/#download. For Qt 5 compatibility, wwWidgets must be built from source.
4. HTTPS requires the OpenSSL binaries. To build OpenSSL using MinGW:
  1. Download and install MSYS2 from http://www.msys2.org/
  2. Update the package database as indicated in instructions
  3. Prepare the MSYS2 build environment as shown here: http://wiki.qt.io/MSYS2
  4. Follow the instructions here to install OpenSSL: http://wiki.qt.io/Compiling_OpenSSL_with_MinGW
  5. Copy libeay32.dll and ssleay32.dll to the FlySight Viewer distribution folder

## Deploying on Mac

1. If this is your first time deploying:
  1. Install MacPorts (http://macports.org) if needed.
  2. In Terminal run `port install mysql55`.
  3. Install Homebrew (http://brew.sh) if needed.
  4. In Terminal run `brew install postgresql`.
2. In Terminal run:
  1. `/path/to/macdeployqt FlySightViewer.app`
  2. `cp -R /path/to/flysight-viewer-qt/macx/* FlySightViewer.app/Contents/Frameworks`
3. Open Disk Utility.
  1. Go to `File > New Image > Blank Image` and create a 450 MB disk image.
5. Copy `FlySightViewer.app` into the new disk image.
6. Eject the disk image.
7. In Disk Utility, go to `Images > Convert`.
  1. Select the new disk image and click Open.
  2. Check that `Image Format` is set to `compressed`.
  3. Click Save.

## Ubuntu Linux

```bash
# Clone the repo
git clone https://github.com/flysight/flysight-viewer-qt.git && cd flysight-viewer-qt

# WSL2 Install Ubuntu Desktop, OpenGL, and setup the DISPLAY
sudo apt install mesa-utils

# Install C++ and Qt5
sudo apt install build-essential qt5-qmake libqt5core5a libqt5printsupport5 libqt5sql5 qtwebengine5-dev libqt5webengine5

# The QT_SELECT may need to go into your ~/.bashrc if you want it to persist, but for this build we can do it like this
export QT_SELECT=qt5

mkdir third-party && cd third-party

# Setup the VLC libs we need
sudo apt install cmake libvlc-dev libvlccore-dev
git clone https://github.com/vlc-qt/vlc-qt.git
cd vlc-qt
git checkout 1.1.1
cmake .
make
sudo make install
sudo ln -s /usr/local/lib/libVLCQtCore.so /usr/lib/libvlc-qt.so
sudo ln -s /usr/local/lib/libVLCQtWidgets.so /usr/lib/libvlc-qt-widgets.so

cd .. # Back to third-party subdirecgtory

# Download wwWidgets source from: http://www.wysota.eu.org/wwwidgets/#download
wget http://www.wysota.eu.org/wwwidgets/wwWidgets-1.0-qt5.tar.gz
tar xvf wwWidgets-1.0-qt5.tar.gz
cd wwWidgets
qmake
make sub-widgets
sudo ln -sr widgets/libwwwidgets4.so /usr/lib/libwwwidgets4.so
sudo ln -sr widgets/libwwwidgets4.so /usr/lib/libwwwidgets4.so.1

# Build FlySightViewer
cd flysight-viewer-qt/src
cp secrets_template.h secrets.h # Modify this file with your Google Maps API key or just leave it as empty string...
mkdir ../build && cd ../build # Create a build folder out of the source tree
qmake ../src
make

# LD_LIBRARY_PATH is needed so that we pick up the VLC libs we didn't copy...
export LD_LIBRARY_PATH=/usr/lib:/usr/lib/x86_64-linux-gnu:/usr/local/lib
# WSL2 Workaround: OpenGL crash https://github.com/microsoft/wslg/issues/1214
export GALLIUM_DRIVER=llvmpipe

# Run the viewer
./FlySightViewer
```

If you have trouble building FlySight Viewer on Linux, check your Qt version number using the following command:

`qmake --version`

FlySight Viewer requires Qt version 5.5. If you have a different version installed, you will need to follow these steps to build with Qt 5.5:

1. Download Qt 5.5 from https://www.qt.io/download-open-source/. Install to `~/Qt`.
2. Edit `FlySightViewer.pro` to add the following lines:
```
LIBS += -L/home/$USER/Qt/5.5/gcc_64/lib
INCLUDEPATH += /home/$USER/Qt/5.5/gcc_64/include
INCLUDEPATH += /home/$USER/Qt/5.5/gcc_64/include/QtCore
```
3. Run `make` again.

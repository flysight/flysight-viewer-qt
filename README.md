# FlySight Viewer

## Libraries

1. FlySight Viewer should be built using Qt 5.5.1. Later versions of Qt do not include QWebView, which is used by MapView.
2. VLC-Qt builds can be found here: https://github.com/vlc-qt/vlc-qt/releases. For Qt 5.5, use VLC-Qt 1.0.
3. wwWidgets is located here: http://www.wysota.eu.org/wwwidgets/#download. For Qt 5 compatibility, wwWidgets must be built from source.

## Deploying on Mac

1. If this is your first time deploying:
  1. Install MacPorts (http://macports.org) if needed.
  2. In Terminal run `port install mysql55`.
  3. Install Homebrew (http://brew.sh) if needed.
  4. In Terminal run `brew install postgresql`.
2. In Terminal run:
  1. `/path/to/macdeployqt FlySightViewer.app`
  2. `cp -R /path/to/flysight-viewer-qt/frameworks/* FlySightViewer.app/Contents/Frameworks`
  3. `cp -R /path/to/flysight-viewer-qt/plugins FlySightViewer.app/Contents/MacOS`
3. Open Disk Utility.
  1. Go to `File > New Image > Blank Image` and create a 250 MB disk image.
5. Copy `FlySightViewer.app` into the new disk image.
6. Eject the disk image.
7. In Disk Utility, go to `Images > Convert`.
  1. Select the new disk image and click Open.
  2. Check that `Image Format` is set to `compressed`.
  3. Click Save.

## Ubuntu Linux

```bash
sudo apt install libqt5webkit5-dev
sudo add-apt-repository ppa:ntadej/tano
sudo apt update
sudo apt install libvlc-qt-dev
sudo cp /usr/lib/libVLCQtCore.so /usr/lib/libvlc-qt.so
sudo cp /usr/lib/libVLCQtWidgets.so /usr/lib/libvlc-qt-widgets.so

export QT_SELECT=qt5

# Download wwWidgets source from: http://www.wysota.eu.org/wwwidgets/#download
cd wwWidgets
qmake
make sub-widgets
sudo cp widgets/libwwwidgets4.so /usr/lib/libwwwidgets4.so
sudo cp widgets/libwwwidgets4.so /usr/lib/libwwwidgets4.so.1

# Build FlySightViewer
cd flysight-viewer-qt/src
qmake
make
```

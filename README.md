# FlySight Viewer

## Deploying on Mac

1. If this is your first time deploying:
  1. Install MacPorts (http://macports.org) if needed.
  2. In Terminal run `port install mysql55`.
  3. Install Homebrew (http://brew.sh) if needed.
  4. In Terminal run `brew install postgresql`.
2. In Terminal run:
  1. `/path/to/macdeployqt FlySightViewer.app`
  2. `cp -R /path/to/flysight-viewer-qt/frameworks/* FlySightViewer.app/Contents/Frameworks`
  3. `cp -R /Applications/VLC.app/Contents/MacOS/plugins FlySightViewer.app/Contents/MacOS`

# Open-Ephys Plugin GUI
## Probe Viewer Plugin

### Contents
* [Description](#description)
* [Building From Source](#build-instructions)
  * [MacOS and Xcode](#build-mac-xcode)
  * [Windows and Visual Studio 201X](#build-win-vs201x)
  * [Linux and Make](#build-linux-make)


### <a id="description"></a>Description
# TODO: Plugin description goes here...


### <a id="build-instructions"></a>Building From Source

Building this plugin also requires downloading and building the plugin-GUI source.

First, clone both the [plugin-GUI](http://github.com/open-ephys/plugin-GUI) and this repository to the same directory. The plugin can automatically find the necessary binary objects from the base application if these two repos are located in the same directory.

Platform-specific instructions are found below:

#### <a id="build-mac-xcode"></a>MacOS and Xcode

To build for MacOS, open the plugin's workspace file ProbeViewerPlugin.xcworkspace. If the base repository is located in the same directory as the plugin-GUI, the workspace should automatically link to the necessary dependencies from the plugin-GUI project. If these are not found or if your plugin-GUI is located in a different directory, the following references will need to be fixed in Xcode's file browser:

* open-ephys.xcodeproj
* ProbeViewerPlugin/Config/Plugin_Debug.xcconfig
* ProbeViewerPlugin/Config/Plugin_Release.xcconfig

`open-ephys.xcodeproj` is located in the plugin-GUI repository at `plugin-GUI/Builds/MacOSX/plugin-gui.xcodeproj`.
`Plugin_Debug.xcconfig` and `Plugin_Release.xcconfig` are both located at `plugin-GUI/Builds/MacOSX/Plugins/Config/`.

Once these dependencies are set correctly, this plugin can be built using the ProbeViewer scheme in Xcode. __Note: Other plugins that ship with plugin-GUI will not be built. If you need to build these as well, you will need to open and build__ `plugin-GUI/Builds/MacOSX/OpenEphys.xcworkspace`.

If everything builds successfully, the plugin will be installed to `~/Library/Application\ Suport/open-ephys/PlugIns/` and the gui application will launch. Your plugin should load automatically from its installation directory.


#### <a id="build-win-vs201x"></a>Windows and Visual Studio 201X


#### <a id="build-linux-make"></a>Linux and Make

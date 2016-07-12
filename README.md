# OSVR-user-settings
A simple QT based utility to help users set up their configuration parameters

The application by default reads and writes a file called osvr_user_settings.json.

The schema is documented in the file user_schema.json.

An example user config file is in the file osvr_user_settings.json. This file gets read/written to the /ProgramData/OSVR directory on Windows platforms.

The application is built using QT and relies on the jsoncpp libraries.

##Deployment requires bundling with a few QT libraries and some other helper executables:
- Qt5Core.dll
- Qt5Gui.dll
- Qt5Widgets.dll
- Qt5SeriaLport.dll
- icudt53.dll
- icuin53.dll
- icuuc53.dll
- libstdc++-6.dll
- libwinpthread-1.dll


##SW Utilities pane provides launching of:
- direct mode enable/disable
- reset yaw application

The method to launch these involves a bat file which should contain the actual locations of the exes. 

##OSVR HDK Utilities pane provides:
- function to check the current FW version of your HDK
- FW update utility to load different FW versions on your HDK. Note that this function relies on the use of an opensource package called dfu-programmer which can be found here: dfu-programmer.github.io.

Occasionally you may experience issues where the HDK is not properly detected using these utilities. This will sometimes require you to unplug/replug in the HDK. You might be required to do this several times before the system corrects itself. The best results are usually experienced if you:
<ol>
<li>exit this utility</li>
<li>unplug power and usb from the belt pack</li>
<li>apply power to the belt pack</li>
<li>re-attach the usb cable to the belt pack</li>
</ol>

##To build the projects
- osvr_config:
Requires the QT environment. Once installed, open the OSVR_config.pro file and the system will build the rest of the application. I used the MINGW compiler.
- com_osvr_user_settings: to build this plugin, follow the same method as building an out of tree osvr plugin as documented on the osvr developer site. You must run CMAKE on the CMakeList.Txt file and this should produce a corresponding VS2012 solution file.
- usersettingsclient.exe: to build this stand alone application, you must first run CMAKE on the CMakeList.Txt file and this should produce a corresponding VS2012 solution file. The application is an OSVR client and will monitor the user settings for IPD, standing height, and seated height. To extend the parameters being pushed through the system, you will have to modify both the plugin and this client application.

##Things on the todo list:
- remove jsoncpp
- continuous integration
- change external program invocations to use an XML config file to promote cross platform compatibility
- integrate device parser application
- add functionality to enable users to configure their systems choosing from the available attached hardware
- integrate osvr_print.exe

##To consume any of these settings, you must have a few other components in place:
###com osvr user settings.dll
- this is a server side plugin that is able to read the settings in the user settings file
- this file must be installed in the osvr-plugins-0 directory of the server binary executable
- the plug in currently assumes the settings file is in the $APPDIR/OSVR directory

###osvr server config.json
- this is the server config file
- for convenience, a properly edited version is distributed with the plugin
- it must be edited to include the user settings plugin.
- adds plugin entry and some aliases

<pre><code>
`"plugins": [
        "com osvr user settings"
    ],
    "aliases": [
        {
            "/me/IPD": "/com_osvr_user_settings/UserSettings/analog/0",
            "/me/StandingHeight": "/com_osvr_user_settings/UserSettings/analog/1",
            "/me/SeatedHeight": "/com_osvr_user_settings/UserSettings/analog/2"
        }
    ]
</code></pre>
	
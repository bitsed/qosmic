Qosmic is graphical interface for creating, editing, and rendering [flam3 fractal images](http://flam3.com/).  Fractal images created with this application can be used to contribute new artwork to the [electricsheep](http://www.electricsheep.org/) distributed computing platform.

![![](http://qosmic.googlecode.com/svn/trunk/images/qosmic-1.5.0-screen0-thumb.jpg)](http://qosmic.googlecode.com/svn/trunk/images/qosmic-1.5.0-screen0.jpg)

## How to try it ##
There are a few GNU/Linux distributions that have pre-built packages, so you should first check if one exists for your distribution.

If you can't find a package, aren't a linux person, or you just really want to compile, then you will first need to have [Qt4](http://qt.nokia.com/), [flam3](http://flam3.com/),  and [Lua](http://www.lua.org/).  For most of the popular GNU/Linux distributions you will need to have the corresponding 'dev' packages installed as well.
  * Install all build dependencies.
  * Download and unpack the source archive, and cd into the source directory.
  * Edit qosmic.pro to configure the qmake build system to suit your environment.
  * Run qmake
  * Run make
  * Run ./qosmic when the build finishes.
  * Study the [README](http://qosmic.googlecode.com/svn/trunk/README) file for tips on usage.

If you have a problem building, then first check the [README](http://qosmic.googlecode.com/svn/trunk/README) to confirm that you have the correct library dependencies.  There is also a [Build and Install](http://code.google.com/p/qosmic/wiki/BuildAndInstall) wiki page containing a section that provides some guidance for Ubuntu users.


## Links ##
  * [flam3.com](http://flam3.com/) - for more information on libflam3
  * [the electric sheep](http://www.electricsheep.org/) collective dream
  * Icons used in Qosmic are part of the [Silk Icon Set](http://www.famfamfam.com/lab/icons/silk/)
  * [Apophysis](http://www.apophysis.org/) - the flam3 editor for windows
  * A list of [changes](http://qosmic.googlecode.com/svn/trunk/changes.txt) made in each version
  * Some Lua [example scripts](http://qosmic.googlecode.com/svn/trunk/scripts/) for use in Qosmic
  * More [screenshots](http://qosmic.googlecode.com/svn/trunk/images/) of older versions.
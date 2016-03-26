# Introduction #

If you have problems building, then this page will hopefully point you in the right direction.  The first thing to do is check the README, and make sure that you have the required libraries installed with their header files.

All of the build control variables are contained in the qosmic.pro file.  To resolve a compile error you should only need to edit qosmic.pro and run 'qmake && make' (assuming
that all of the required libraries are installed).

The qosmic.pro file is setup to use pkg-config by default.  If you don't have
pkg-config, or if it doesn't work on your system, then you'll need to edit the
qosmic.pro file to set the required library and include paths.  There are some
lines in the file that should need only some minor path changes for this to set
these for your system.

Many linux packaging systems will store the header files for libraries in
another package which are 'dev' packages (ie qt-4.3.1-dev.rpm).  These header
files are needed to build qosmic, so make sure the dev packages for the
required libraries are installed if they exist.

When using Debian Sid one must manually link qmake correctly against qmake-qt4
before compiling.  This may be a problem on other systems as well, so make sure
the qmake in your path is the one installed by Qt4.  You can run the following
command to relink qmake on Debian Sid before running build.sh.
```
$ sudo update-alternatives --config qmake
```

For the Lua dependency it may be necessary to adjust the include list in
qosmic.pro so the liblua header files can be found.  You may need to add
/usr/include/lua5.1 to the INCLUDEPATH variable.  You may also need to change
-llua in the LIBS variable to -llua5.1 and/or add the path of the library file
to the LIBS variable using the '-L/path/to liblua.so/' option depending on how
your distribution installed Lua.

Setting the system specific locations of libraries and their header files can be
a problem with any of the required dependencies.  The instructions for finding
the Lua libraries can also be used to find any other library's install points
(ie flam3).  Looking in qosmic.pro you'll notice that the INCLUDEPATH lists
/usr/include/libxml2 which is where the libxml2 header files are located on my
system.

If you see an error about qosmic being unable to find the flam3-palettes.xml
file, then you need to adjust the FLAM3\_PALETTES variable in qosmic.pro so that
it points at the directory where flam3 installed this file.  Alternatively, you
can use the environment variable 'flam3\_palettes' to do this.  Type './qosmic
--help' to see a list of supported environment variables.


---


# Compiling from source on Ubuntu 9.04 #

Use these steps to compile and install from the source on Ubuntu 9.04.  A similar set of
steps can be taken for other versions of Ubuntu as well.

1. Add the flam3 PPA to your list of repositories.

```
sudo apt-key adv --recv-keys --keyserver keyserver.ubuntu.com 'f191582f22b943d1029b2d5a91dae98f32ffb679'
sudo echo "deb http://ppa.launchpad.net/flam3/ppa/ubuntu intrepid main" >> /etc/apt/sources.list 
sudo apt-get update
```


2. Install development tools and libraries.

```
sudo apt-get install build-essential qt4-dev-tools liblua5.1-0-dev libpng12-dev libjpeg62-dev libxml2-dev flam3
```


3. Download and unpack the Qosmic source.  To configure, edit qosmic.pro, and adjust the `PKGCONFIG` variable to use lua5.1 by making the following change.

```
PKGCONFIG = flam3 lua5.1
```


4. Versions earlier than 1.4.6 don't include a qosmic.desktop file.  To install a menu item in the applications menu for earlier versions, first add the following below `CONFIG += install_icons` in qosmic.pro.

```
CONFIG += install_desktop

install_desktop {
   desktop.files = qosmic.desktop
   desktop.path  = $$SHARED/applications
   INSTALLS += desktop
}
```


Next save the following to a new file named `qosmic.desktop` in your top level qosmic source directory.

```
[Desktop Entry]
Type=Application
Name=Qosmic
GenericName=Flam3 Fractal Editor
Exec=qosmic
Icon=qosmicicon
Terminal=false
Categories=Qt;KDE;Graphics;
```

5. Compile and install.

```
qmake && make && make install ;
```

6. Type `/usr/bin/qosmic` to run, or use the Applications->Graphics->Qosmic menu item.



---


# FAQ #


**Q:** I get the following error from qmake, and I'm certain that I have Qt4 installed
with it's -dev packages.
```
Building Qosmic
Project ERROR: qosmic requires Qt 4
make: *** No targets specified and no makefile found.  Stop.
```

**A:**
You probably have Qt3 libraries installed as  well, and the qmake that the build
script is running (the one in your path) is for Qt3.  You could
try putting the full path to the Qt4 qmake executable in the build script, or
just run it from the command line followed by 'make'.


**Q:** I get the following error from qmake
```
$ qmake
Project MESSAGE: Generating Makefile for Qosmic version 1.4.2
Project MESSAGE: Qt version : 4.3.4
Project MESSAGE: Default number of rendering threads : 2
Project MESSAGE: Location of flam3-palettes.xml : /usr/local/share/flam3
Package lua was not found in the pkg-config search path.
Perhaps you should add the directory containing `lua.pc'
to the PKG_CONFIG_PATH environment variable
```

**A:**
It looks like the lua-dev package didn't install the pkg-config file.

You can adjust the paths in the qosmic.pro file to point at your build
dependencies.  Try removing the lua entry from the PKGCONFIG variable,
then try and locate lua.h somewhere in /usr/include/`*`, and add that directory
to the INCLUDEPATH, and if liblua.so.5 is somewhere strange (other than
/usr/lib) then add that path to the LIBS variable.  You will also need to add
'-llua' to the LIBS variable.
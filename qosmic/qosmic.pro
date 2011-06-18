################################################################################
#################### qmake project file for qosmic application #################
################################################################################
## Installation prefix on *nix/osx.  Binaries are installed in $$PREFIX/bin
PREFIX = /usr


################################################################################
## The shared resources directory
SHARED = $$PREFIX/share


################################################################################
## The translations files are installed in $$SHARED/translations
TRANSDIR = $$SHARED/translations


################################################################################
## Icons are installed in $$SHARED/pixmaps
ICONSDIR = $$SHARED/pixmaps


################################################################################
## Qosmic Lua scripts are installed in $$SHARED/scripts
SCRIPTSDIR = $$SHARED/scripts


################################################################################
## Install the qosmic.desktop file here.
DESKDIR = $$SHARED/applications


################################################################################
## Uncomment to install the qosmic.desktop file and the application icons.
CONFIG += install_icons install_desktop


################################################################################
## Add your non-english locale here to install the translations.  Only a naive
## set of french translations is available.
#CONFIG += ts_fr


################################################################################
## Add linked libs and paths for headers and palettes here using pkg-config.
## If your system doesn't support pkg-config then comment out the next line and
## set these values below.
#CONFIG += link_pkgconfig

link_pkgconfig {
	message("Config using pkg-config version "$$system(pkg-config --version))
	PKGCONFIG = flam3 lua

	## The directory that contains flam3-palettes.xml must be set here.  If your
	## system has pkg-config this should find the palettes.
	PALETTESDIR = $$system(pkg-config --variable=datarootdir flam3)/flam3
}
else {
	message("Config not using pkg-config")
	## Adjust these variables to set paths and libs without using pkg-config.
	## The PALETTESDIR must be set to the directory containing the
	## flam3-palettes.xml file installed by the flam3 package.

	## Point to the dir containing the compiled flam3-2.8 if it's not installed
	FLAM3_SRC_DIR = $$system(readlink -e ../flam3-3.0)
	PALETTESDIR = $$FLAM3_SRC_DIR
	INCLUDEPATH += $$FLAM3_SRC_DIR /usr/include/libxml2
	LIBS += -L$$FLAM3_SRC_DIR/.libs -L/usr/lib/libxml2 -lflam3 -lm -ljpeg -lxml2 -llua
}


################################################################################
## Build style flags.  Adding debug enables more verbose logging.
# CONFIG += release warn_off
#CONFIG += debug warn_on


################################################################################
## Set cflags here if needed.
#QMAKE_CFLAGS="-march=core2 -O2 -pipe -Wl,-t"
#QMAKE_CXXFLAGS=$$QMAKE_CFLAGS


################################################################################
## qosmic app version
VERSION = 1.5.0_alpha2

################################################################################
## Check for correct package versions
system( test $$QT_MINOR_VERSION -lt 5 ) {
	error("Using Qt $$[QT_VERSION]. " \
	"Qosmic $$VERSION requires at least version 4.5 of Qt to build.")
}

link_pkgconfig {
	! system(pkg-config --atleast-version 2.8 flam3) {
		error("Qosmic $$VERSION requires at least version 2.8 of flam3 to build.")
	}
}

! exists($$PALETTESDIR/flam3-palettes.xml) {
	error("The file $$PALETTESDIR/flam3-palettes.xml doesn't exist. " \
	"Please install libflam3 (http://flam3.org/) and set PALETTESDIR" \
	"to the directory containing flam3-palettes.xml in qosmic.pro.")
}

################################################################################
DEFINES += VERSION='\'"$$VERSION"\''
DEFINES += FLAM3DIR='\'"$$PALETTESDIR"\''
DEFINES += TRANSDIR='\'"$$TRANSDIR"\''
DEFINES += SCRIPTSDIR='\'"$$SCRIPTSDIR"\''
CONFIG += qt thread uitools
RESOURCES = qosmic.qrc
INCLUDEPATH += src
DESTDIR = .

## add the target to the install set
target.path += $$PREFIX/bin
INSTALLS += target

## add the translations to the install set
ts_fr {
	translations.files= ts/*.qm
	translations.path = $$TRANSDIR
	INSTALLS += translations
}

## add icons to the install set
install_icons {
	icons.files = icons/qosmicicon.xpm
	icons.path = $$ICONSDIR
	INSTALLS += icons
}

## add the qosmic.desktop file to the install set
install_desktop {
   desktop.files = qosmic.desktop
   desktop.path  = $$DESKDIR
   INSTALLS += desktop
}

message(Generating Makefile for Qosmic version $$VERSION)
message(Qt version : $$[QT_VERSION])
message(Location of flam3-palettes.xml : $$PALETTESDIR)
! link_pkgconfig {
	message(Include header paths : $$INCLUDEPATH)
	message(Include libraries : $$LIBS)
}

CONFIG(debug, debug|release) {
	## enable more verbose logging
	message(Building debug version)
	DEFINES += LOGGING
}

FORMS += \
 ui/mainwindow.ui \
 ui/mainviewer.ui \
 ui/paletteeditor.ui \
 ui/camerasettingswidget.ui \
 ui/colorsettingswidget.ui \
 ui/imgsettingswidget.ui \
 ui/mainpreviewwidget.ui \
 ui/trianglecoordswidget.ui \
 ui/triangledensitywidget.ui \
 ui/variationswidget.ui \
 ui/mutationwidget.ui \
 ui/colorbalancewidget.ui \
 ui/directoryviewwidget.ui \
 ui/statuswidget.ui \
 ui/scripteditwidget.ui \
 ui/colordialog.ui \
 ui/selectgenomewidget.ui \
 ui/genomevectorlistmodelitemeditor.ui \
 ui/selectgenomeconfigdialog.ui \
 ui/mutationconfigdialog.ui \
 ui/viewerpresetswidget.ui \
 ui/selecttrianglewidget.ui \
 ui/renderdialog.ui \
 ui/renderprogressdialog.ui \
 ui/adjustscenewidget.ui \
 ui/editmodeselectorwidget.ui \
 ui/chaoswidget.ui \
 ui/sheeploopwidget.ui

HEADERS += \
 src/qosmic.h \
 src/flam3util.h \
 src/mainwindow.h \
 src/xfedit.h \
 src/basistriangle.h \
 src/triangle.h \
 src/nodeitem.h \
 src/mainviewer.h \
 src/renderthread.h \
 src/colorselector.h \
 src/genomecolorselector.h \
 src/paletteeditor.h \
 src/mutationwidget.h \
 src/colorlabel.h \
 src/doublevalueeditor.h \
 src/intvalueeditor.h \
 src/camerasettingswidget.h \
 src/colorsettingswidget.h \
 src/imgsettingswidget.h \
 src/mainpreviewwidget.h \
 src/trianglecoordswidget.h \
 src/variationswidget.h \
 src/logger.h \
 src/colorbalancewidget.h \
 src/gradientlistmodel.h \
 src/varstablewidget.h \
 src/directoryviewwidget.h \
 src/flamfileiconprovider.h \
 src/directorylistview.h \
 src/snapslider.h \
 src/statuswidget.h \
 src/scripteditwidget.h \
 src/wheelvalueeditor.h \
 src/genomevector.h \
 src/lua/lunar.h \
 src/lua/frame.h \
 src/lua/xform.h \
 src/lua/genome.h \
 src/lua/luathread.h \
 src/colordialog.h \
 src/selectgenomewidget.h \
 src/viewerpresetsmodel.h \
 src/viewerpresetswidget.h \
 src/coordinatemark.h \
 src/lua/luathreadadapter.h \
 src/lua/highlighter.h \
 src/lua/luaeditor.h \
 src/lua/luatype.h \
 src/selecttrianglewidget.h \
 src/triangledensitywidget.h \
 src/undoring.h \
 src/triangleselection.h \
 src/posttriangle.h \
 src/qosmicwidget.h \
 src/renderdialog.h \
 src/renderprogressdialog.h \
 src/adjustscenewidget.h \
 src/gradientstopseditor.h \
 src/editmodeselectorwidget.h \
 src/genomevectorlistview.h \
 src/chaoswidget.h \
 src/transformablegraphicsitem.h \
 src/transformablegraphicsguide.h \
 src/sheeploopwidget.h

SOURCES += \
 src/qosmic.cpp \
 src/mainwindow.cpp \
 src/xfedit.cpp \
 src/basistriangle.cpp \
 src/triangle.cpp \
 src/nodeitem.cpp \
 src/flam3util.cpp \
 src/mainviewer.cpp \
 src/renderthread.cpp \
 src/colorselector.cpp \
 src/genomecolorselector.cpp \
 src/paletteeditor.cpp \
 src/logger.cpp \
 src/mutationwidget.cpp \
 src/colorlabel.cpp \
 src/doublevalueeditor.cpp \
 src/intvalueeditor.cpp \
 src/camerasettingswidget.cpp \
 src/colorsettingswidget.cpp \
 src/imgsettingswidget.cpp \
 src/mainpreviewwidget.cpp \
 src/trianglecoordswidget.cpp \
 src/variationswidget.cpp \
 src/colorbalancewidget.cpp \
 src/gradientlistmodel.cpp \
 src/varstablewidget.cpp \
 src/directoryviewwidget.cpp \
 src/flamfileiconprovider.cpp \
 src/directorylistview.cpp \
 src/snapslider.cpp \
 src/statuswidget.cpp \
 src/scripteditwidget.cpp \
 src/wheelvalueeditor.cpp \
 src/lua/frame.cpp \
 src/lua/genome.cpp \
 src/lua/xform.cpp \
 src/lua/luathread.cpp \
 src/colordialog.cpp \
 src/selectgenomewidget.cpp \
 src/genomevector.cpp \
 src/viewerpresetsmodel.cpp \
 src/viewerpresetswidget.cpp \
 src/coordinatemark.cpp \
 src/lua/luathreadadapter.cpp \
 src/lua/highlighter.cpp \
 src/lua/luaeditor.cpp \
 src/lua/luatype.cpp \
 src/selecttrianglewidget.cpp \
 src/triangledensitywidget.cpp \
 src/undoring.cpp \
 src/triangleselection.cpp \
 src/posttriangle.cpp \
 src/qosmicwidget.cpp \
 src/renderdialog.cpp \
 src/renderprogressdialog.cpp \
 src/adjustscenewidget.cpp \
 src/gradientstopseditor.cpp \
 src/editmodeselectorwidget.cpp \
 src/genomevectorlistview.cpp \
 src/chaoswidget.cpp \
 src/transformablegraphicsitem.cpp \
 src/transformablegraphicsguide.cpp \
 src/sheeploopwidget.cpp


TRANSLATIONS = ts/qosmic_fr.ts

MOC_DIR = .moc
OBJECTS_DIR = .obj
RCC_DIR = .res
UI_DIR = .ui

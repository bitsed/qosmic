################################################################################
#################### qmake project file for qosmic application #################
################################################################################
## Installation prefix on *nix/osx.  Binaries are installed in $$PREFIX/bin
PREFIX = /usr


################################################################################
## The shared resources directory
SHARED = $$PREFIX/share


################################################################################
## Icons are installed in $$SHARED/pixmaps
ICONSDIR = $$SHARED/pixmaps


################################################################################
## Qosmic Lua scripts are installed in $$SHARED/scripts
SCRIPTSDIR = $$SHARED/qosmic/scripts


################################################################################
## Install the qosmic.desktop file here.
DESKDIR = $$SHARED/applications


################################################################################
## Uncomment to install the qosmic.desktop file, application icons, and scripts.
CONFIG += install_icons install_desktop install_scripts


################################################################################
## Add linked libs and paths for headers and palettes here using pkg-config.
## If your system doesn't support pkg-config then comment out the next line and
## set these values below.
#CONFIG += link_pkgconfig

link_pkgconfig {
	message("Config using pkg-config version "$$system(pkg-config --version))
	PKGCONFIG = flam3 lua5.2
}
else {
	message("Config not using pkg-config")
	## Adjust these variables to set paths and libs without using pkg-config.
	## Point to the flam3-3.1.1 source directory
	FLAM3_SRC_DIR = $$system(readlink -e ../flam3-3.1.1)
	INCLUDEPATH += $$FLAM3_SRC_DIR /usr/include/libxml2 /usr/include/lua5.2/
	LIBS += -L$$FLAM3_SRC_DIR/.libs
	LIBS += -L/usr/lib/libxml2 -lflam3 -lm -ljpeg -lxml2 -llua5.2
}


################################################################################
## Build style flags.  Adding debug enables more verbose logging.
CONFIG += release warn_off
#CONFIG += debug warn_on


################################################################################
## Set cflags here if needed.
#CONFIG(release, debug|release) {
#	QMAKE_CFLAGS="-march=native -O2 -pipe -Wl,-t"
#	QMAKE_CXXFLAGS=$$QMAKE_CFLAGS
#}


################################################################################
## qosmic app version
VERSION = 1.6.0

################################################################################
## Check for correct package versions
system(test $$QT_MAJOR_VERSION -lt 5 -o $$QT_MINOR_VERSION -lt 5) {
	error("Using Qt $$[QT_VERSION]. " \
	"Qosmic $$VERSION requires at least version 5.5 of Qt to build.")
}

link_pkgconfig {
	! system(pkg-config --atleast-version 3.1.1 flam3) {
		error("Qosmic $$VERSION requires at least version 3.1.1 of flam3 to build.")
	}
}

################################################################################
DEFINES += VERSION='\'"$$VERSION"\''
DEFINES += SCRIPTSDIR='\'"$$SCRIPTSDIR"\''
CONFIG += qt thread
QT += widgets
RESOURCES = qosmic.qrc
INCLUDEPATH += src

## packagers may define ROOT for install sandboxes
#ROOT = /tmp

## add the qosmic program to the install set
qosmic.files = qosmic
qosmic.path = $$clean_path($$ROOT/$$PREFIX/bin)
INSTALLS = qosmic

## add icons to the install set
install_icons {
	icons.files = icons/qosmicicon.xpm
	icons.path = $$clean_path($$ROOT/$$ICONSDIR)
	INSTALLS += icons
}

## add the qosmic.desktop file to the install set
install_desktop {
   desktop.files = qosmic.desktop
   desktop.path  = $$clean_path($$ROOT/$$DESKDIR)
   INSTALLS += desktop
}

## add the scripts to the install set
install_scripts {
   scripts.files = scripts/*
   scripts.path  = $$clean_path($$ROOT/$$SCRIPTSDIR)
   INSTALLS += scripts
}

message(Generating Makefile for Qosmic $$VERSION)
message(Qt version : $$[QT_VERSION])
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
 ui/sheeploopwidget.ui \
 ui/scripteditconfigdialog.ui

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
 src/sheeploopwidget.h \
 src/flam3filestream.h \
 src/checkersbrush.h

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
 src/sheeploopwidget.cpp \
 src/flam3filestream.cpp \
 src/checkersbrush.cpp


TRANSLATIONS = ts/qosmic_fr.ts \
 ts/qosmic_cs.ts \
 ts/qosmic_ru.ts

MOC_DIR = .moc
OBJECTS_DIR = .obj
RCC_DIR = .res
UI_DIR = .ui

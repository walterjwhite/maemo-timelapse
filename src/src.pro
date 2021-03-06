#-----------------------------
# QMake build file for fcamera

TEMPLATE = app
TARGET = timelapse

# If you're compiling fcamera without installing fcam-dev,
# (so either you're using it from the Nokia Qt SDK/MADDE, or you just downloaded the zip into scratchbox)
# rename fcamera.local.template.pro to fcamera.local.pro, and adjust its paths if your organization doesn't match
# the fcam Maemo-garage repository defaults
!include( timelapse.local.pro ) {
  message("!! No timelapse.local.pro found, assuming fcam-dev is installed. If this isn't true, please copy fcamera.local.template.pro to fcamera.local.pro, and adjust the path to point to where your fcam files are, if you're not using the default Maemo garage organization for FCam.")
}

#-----------------------------
# Source and header files

HEADERS += FCamera.h
SOURCES += FCamera.cpp 

#-----------------------------
# Source and header paths

VPATH += src
DEPENDPATH += .

#-----------------------------
# Libraries

LIBS += -lpthread -lFCam -ljpeg

#-----------------------------
# Build destination paths
OBJECTS_DIR = ../build
MOC_DIR     = ../build
UI_DIR      = ../build

DESTDIR     = ../build

#-----------------------------
# QT Configuration

CONFIG += release warn_on
QT +=
CONFIG += qt
QMAKE_CXXFLAGS += -mfpu=neon -mfloat-abi=softfp


#-----------------------------
# Packaging setup

PREFIX = ../debian/timelapse/usr
BINDIR = $$PREFIX/bin
DATADIR =$$PREFIX/share
 
DEFINES += DATADIR=\"$$DATADIR\" PKGDATADIR=\"$$PKGDATADIR\"
 
INSTALLS    += target
target.path  = $$BINDIR

INSTALLS    += desktop
desktop.path  = $$DATADIR/applications/hildon
desktop.files  = ../data/timelapse.desktop

INSTALLS    += icon64
icon64.path  = $$DATADIR/pixmaps
icon64.files  = ../data/icons/64x64/timelapse.png


#
# Targets for debian source and binary package creation

debian-src.commands = dpkg-buildpackage -S -r -us -uc -d -I'\.svn';
debian-src.commands += mv ../$(QMAKE_TARGET)_*{.dsc,.tar.gz,_source.changes} .
debian-bin.commands = dpkg-buildpackage -b -r -uc -d;
debian-bin.commands += mv ../$(QMAKE_TARGET)_*_armel.{deb,changes} .

#debian-all.depends = debian-src debian-bin

#-----------------------------
# Clean all but Makefile

compiler_clean.commands = -$(DEL_FILE) $(TARGET)

#-----------------------------
# MADDE remote commands

go.commands  = mad remote -r N900 send $(TARGET); 
go.commands += mad remote -r N900 run $(QMAKE_TARGET)
go.depends = $(TARGET)

gobug.commands = mad remote -r N900 send $(TARGET); 
gobug.commands += mad remote -r N900 debug $(QMAKE_TARGET)
gobug.depends = $(TARGET)

#-----------------------------
# Add all of the xtra QMAKE targets from above

QMAKE_EXTRA_TARGETS += debian-all debian-src debian-bin compiler_clean go gobug

OTHER_FILES +=

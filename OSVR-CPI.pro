#-------------------------------------------------
#
# Project created by QtCreator 2015-06-04T13:52:08
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OSVR_CPI
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    osvruser.cpp \
    lib_json/json_reader.cpp \
    lib_json/json_value.cpp \
    lib_json/json_writer.cpp \
    firmwareupdateprogressdialog.cpp

HEADERS  += mainwindow.h \
    osvruser.h \
    json/assertions.h \
    json/autolink.h \
    json/config.h \
    json/features.h \
    json/forwards.h \
    json/json.h \
    json/reader.h \
    json/value.h \
    json/version.h \
    json/writer.h \
    lib_json/json_batchallocator.h \
    lib_json/json_tool.h \
    version.h \
    firmwareupdateprogressdialog.h

FORMS    += mainwindow.ui \
    firmwareupdateprogressdialog.ui

DISTFILES += \
    resources.rc \
    logo.ico

# Note: the version the executable gets is from version.h, not here
VERSION = "0.0.1.0"
QMAKE_TARGET_COMPANY = OSVR
QMAKE_TARGET_PRODUCT = OSVR_CPI
QMAKE_TARGET_DESCRIPTION = "OSVR Control Panel Interface"
QMAKE_TARGET_COPYRIGHT = "Copyright 2016 OSVR and contributors"

RC_FILE = resources.rc

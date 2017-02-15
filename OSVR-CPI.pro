# Copyright 2016 OSVR and contributors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HDK_Firmware_Utility
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
QMAKE_TARGET_PRODUCT = HDK_Firmware_Utility
QMAKE_TARGET_DESCRIPTION = "HDK Firmware Utility"
QMAKE_TARGET_COPYRIGHT = "Copyright 2016 OSVR and contributors"

RC_FILE = resources.rc

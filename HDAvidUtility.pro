QT       += core gui

TARGET = HDAvidUtility
TEMPLATE = app


SOURCES += main.cpp\
        HDAvidUtility.cpp

HEADERS  += HDAvidUtility.h

FORMS    += HDAvidUtility.ui

RESOURCES += \
    HDAvidUtility.qrc

win32:RC_FILE = HDAvidUtility.rc

include(../qt-solutions/qtsingleapplication/src/qtsingleapplication.pri)


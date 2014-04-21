#-------------------------------------------------
#
# Project created by QtCreator 2012-08-05T10:42:26
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = adl_sample_app
TEMPLATE = app


SOURCES += main.cpp\
    cdosampleappwindow.cpp \
    renderingwidget.cpp \
    cloudeoctrl.cpp \
    cdohelpers.cpp \
    appcontroller.cpp \
    sha256.c

HEADERS  += cdosampleappwindow.h \
    renderingwidget.h \
    cloudeoctrl.h \
    cdohelpers.h \
    appcontroller.h \
    addlivesdkparams.h \
    sha256.h

FORMS    += cdosampleappwindow.ui


win32: ADL_HOME = $$PWD/../AddLive_sdk-win
unix:!macx {
    ADL_HOME = $$PWD/../AddLive_sdk-linux
}

unix:macx {
    ADL_HOME = $$PWD/../AddLive_sdk-mac
}


win32: LIBS += -L$$ADL_HOME -ladl_sdk -lyuv
unix: {
    LIBS += -L$$ADL_HOME -ladl_sdk -lyuv -ljpeg_turbo
}

INCLUDEPATH += $$ADL_HOME

DEFINES += ADL_DIRECT_RENDERING=1

#QMAKE_CXXFLAGS += -fsanitize=thread -fPIE
#QMAKE_LFLAGS += -pie -fsanitize=thread



#-------------------------------------------------
#
# Project created by QtCreator 2012-08-05T10:42:26
#
#-------------------------------------------------

QT       += core gui

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


LIBS += -L$$ADL_HOME -ladl_sdk
unix:!macx {
    LIBS += -L$$ADL_HOME -L/home/dmitry/smlibs/libs  -ladl_sdk_logic -ladl_stub -lsmplogic -lsmpmedia  -lsmpcommunication  -lsmcommon  -ljsoncpp_common\
        -lboost_system -lboost_serialization -lboost_log -lboost_program_options -lboost_date_time\
        -lboost_chrono -lboost_thread -lboost_filesystem -lprotobuf-lite -lwebrtc_voice -lvorbisenc -logg -lswscale -lavutil -lortp -lvpx -lwebm -lcrypto -lcurl
    INCLUDEPATH += $$(CLOUDEO_LIBS_HOME)/include
}

INCLUDEPATH += $$ADL_HOME

#QMAKE_CXXFLAGS += -fsanitize=thread -fPIE
#QMAKE_LFLAGS += -pie -fsanitize=thread



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


LIBS += -L$$ADL_HOME -ladl_sdk
unix:!macx {
    LIBS += -L$$ADL_HOME -ladl_sdk_logic -ladl_stub -lsmplogic -lsmpmedia  -lsmpcommunication  -lsmcommon  -ljsoncpp_common\
        -lboost_system -lboost_log -lboost_program_options -lboost_date_time\
        -lboost_thread -lboost_chrono -lboost_filesystem -lprotobuf-lite -lwebrtc_voice -lcurl -lssl -lcrypto -lyuv -lvpx\
        -lstun -lsrtp -ldl -lX11
#        -lvorbisenc -logg -lswscale -lavutil -lortp -lwebm
#       -lboost_serialization
#    INCLUDEPATH += $$(CLOUDEO_LIBS_HOME)/include
}

INCLUDEPATH += $$ADL_HOME

#QMAKE_CXXFLAGS += -fsanitize=thread -fPIE
#QMAKE_LFLAGS += -pie -fsanitize=thread



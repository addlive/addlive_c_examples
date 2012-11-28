#include "appcontroller.h"

#include <boost/bind.hpp>
#include <QDebug>
#include <QTime>
#include <cdohelpers.h>
#include <time.h>

#include <sstream>

namespace
{
QVariantMap devsMap2QVariantMap(const std::map<std::string,std::string> in);
void nop(){}

// Dev layer streamer
const std::string gStreamerBase = "174.127.76.179:443/";
}

AppController::AppController(QObject *parent) :
    QObject(parent), _connected(false)
{
}

void AppController::initADL()
{
    qDebug() << "Initializing ADL";
    ADLReadyHandler rH = boost::bind(&AppController::onCdoReady, this, _1, _2);
    _cdoCtrl.initPlatform(rH);
}

void AppController::connect(QString scopeId, bool pAudio, bool pVideo)
{
    qDebug() << "Establishing a connection to scope with id: " << scopeId;
    ADLConnectionDescriptor descr;
    memset(&descr, 0, sizeof(descr));
    descr.autopublishAudio = pAudio;
    descr.autopublishVideo = pVideo;
    QTime m(0,0,0);
    qsrand(m.secsTo(QTime::currentTime()));
    int uId = qrand() % 1000;
    descr.authDetails.userId = uId;
    descr.authDetails.salt = ADLHelpers::stdString2ADLString(
                "Some Salt");
    descr.authDetails.expires = time(NULL) + 300000;
    descr.scopeId = ADLHelpers::stdString2ADLString(scopeId.toStdString());

    descr.url = ADLHelpers::stdString2ADLString(
                gStreamerBase + scopeId.toStdString());

    descr.lowVideoStream.maxBitRate = 64;
    descr.lowVideoStream.maxFps = 5;
    descr.lowVideoStream.maxHeight = 240;
    descr.lowVideoStream.maxWidth = 320;
    descr.lowVideoStream.publish = true;
    descr.lowVideoStream.receive = true;

    descr.highVideoStream.maxBitRate = 512;
    descr.highVideoStream.maxWidth = 640;
    descr.highVideoStream.maxHeight = 480;
    descr.highVideoStream.maxFps = 24;
    descr.highVideoStream.publish = true;
    descr.highVideoStream.receive = true;
    _scopeId = scopeId.toStdString();
    ADLConnectedHandler rh = boost::bind(&AppController::onConnected, this, _1);
    _cdoCtrl.connect(rh, &descr, scopeId.toStdString());
}

void AppController::onUserEvent(void* opaque, const ADLUserStateChangedEvent* e)
{
    ((AppController*) opaque)->onUserEvent(e);

}

void AppController::onMediaEvent(void* opaque,
                                 const ADLUserStateChangedEvent* e)
{
    ((AppController*) opaque)->onMediaEvent(e);
}


/**
  * Slots
  ******************************************************************************
  */


void AppController::playTestSndClicked()
{
    qDebug() << "Playing test sound";
    _cdoCtrl.playTestSound();
}

void AppController::disconnectBtnClicked()
{
    qDebug() << "Terminating connection";
    _cdoCtrl.disconnect(_scopeId);
    _connected = false;
}


void AppController::videoPublishStateChanged(bool state)
{
    if(_connected)
    {
        if(state)
        {
            qDebug() << "Publishing video";
            _cdoCtrl.publish(_scopeId, ADL_MEDIA_TYPE_VIDEO);
        }
        else
        {
            qDebug() << "Unpublishing video";
            _cdoCtrl.unpublish(_scopeId, ADL_MEDIA_TYPE_VIDEO);
        }
    }
}

void AppController::audioPublishStateChanged(bool state)
{
    if(_connected)
    {
        if(state)
        {
            qDebug() << "Publishing audio";
            _cdoCtrl.publish(_scopeId, ADL_MEDIA_TYPE_AUDIO);
        }
        else
        {
            qDebug() << "Unpublishing audio";
            _cdoCtrl.unpublish(_scopeId, ADL_MEDIA_TYPE_AUDIO);
        }
    }
}



/**
  * Callbacks
  ******************************************************************************
  */

void AppController::onCdoReady(ADLH pH, std::string version)
{
    QString qVersion = QString::fromStdString(version);
    qDebug() << "ADL Initialized, version: " << qVersion <<
                ". Continuing with initialization";
    emit cdoReady(pH, qVersion);

    ADLServiceListener listener;
    memset(&listener,0,sizeof(listener));
    listener.onUserEvent = &AppController::onUserEvent;
    listener.onMediaStreamEvent = &AppController::onMediaEvent;
    listener.opaque = this;
    _cdoCtrl.addPlatformListener(&listener);

    ADLDevsHandler rH = boost::bind(&AppController::onVideoDevices, this, _1,
                                    true);
    _cdoCtrl.getVideoCaptureDeviceNames(rH);

    rH = boost::bind(&AppController::onAudioCaptureDevices, this, _1, true);
    _cdoCtrl.getAudioCaptureDeviceNames(rH);

    rH = boost::bind(&AppController::onAudioOutputDevices, this, _1, true);
    _cdoCtrl.getAudioOutputDeviceNames(rH);
}

void AppController::onVideoDevices(std::map<std::string,std::string> devs,
                                   bool firstRun)
{
    qDebug() << "Got video devices list containing " << devs.size() << " items";
    emit mediaDevicesListChanged(VIDEO_IN, devsMap2QVariantMap(devs));
    if(firstRun)
    {
        qDebug() << "Setting video capture device";
        ADLSetDevHandler rh =
                boost::bind(&AppController::onVideoDeviceSet, this, firstRun);
        _cdoCtrl.setVideoCaptureDevice(rh, devs.begin()->first);
    }
}

void AppController::onAudioCaptureDevices(
        std::map<std::string,std::string> devs,bool firstRun)
{
    qDebug() << "Got video devices list containing " << devs.size() << " items";
    emit mediaDevicesListChanged(AUDIO_IN, devsMap2QVariantMap(devs));
    if(firstRun)
    {
        qDebug() << "Setting audio capture device";
        _cdoCtrl.setAudioCaptureDevice(&nop, devs.begin()->first);
    }
}

void AppController::onAudioOutputDevices(
        std::map<std::string,std::string> devs, bool firstRun)
{
    qDebug() << "Got video devices list containing " << devs.size() << " items";
    emit mediaDevicesListChanged(AUDIO_OUT, devsMap2QVariantMap(devs));
    if(firstRun)
    {
        qDebug() << "Setting audio output device";
        _cdoCtrl.setAudioOutputDevice(&nop, devs.begin()->first);
    }
}

void AppController::onVideoDeviceSet(bool startLocalVideo)
{
    if(!startLocalVideo)
        return;
    qDebug() << "Video device configured; Starting local preview";
    ADLLocalVideoStartedHandler rH =
            boost::bind(&AppController::onLocalVideoStarted, this, _1);
    _cdoCtrl.startLocalVideo(rH);
}

void AppController::onLocalVideoStarted(std::string sinkId)
{
    QString qSinkId = QString::fromStdString(sinkId);
    qDebug() << "Local preview started; Sink id: " << qSinkId;
    emit localVideoSinkChanged(qSinkId);
}

void AppController::onConnected(bool succ)
{
    qDebug() << "Got connected result: " << succ;
    _connected = true;
    //emit connected();
}


void AppController::onUserEvent(const ADLUserStateChangedEvent* e)
{
    qDebug() << "Got new user event";
    if(e->isConnected && e->videoPublished)
        emit remoteVideoSinkChanged(
                    ADLHelpers::ADLString2QString(&(e->videoSinkId)));
    else
        emit remoteVideoSinkChanged(QString());

}

void AppController::onMediaEvent(const ADLUserStateChangedEvent* e)
{
    qDebug() << "Got new media event, related to media type: " <<
                ADLHelpers::ADLString2QString(&(e->mediaType));
    if(ADLHelpers::stringEq(&(e->mediaType), ADL_MEDIA_TYPE_VIDEO))
    {
        QString sinkId = e->videoPublished ?
                    ADLHelpers::ADLString2QString(&(e->videoSinkId)) :
                    QString();
        emit remoteVideoSinkChanged(sinkId);
    }
}


namespace
{
QVariantMap devsMap2QVariantMap(const std::map<std::string,std::string> devs)
{
    QVariantMap qDevs;
    std::pair<std::string, std::string> itm;
    foreach(itm, devs)
    {
        qDevs[QString::fromUtf8(itm.first.c_str())] =
                QString::fromUtf8(itm.second.c_str());
    }
    return qDevs;

}
}

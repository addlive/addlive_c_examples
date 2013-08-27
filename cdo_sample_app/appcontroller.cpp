#include <appcontroller.h>
#include <addlivesdkparams.h>
#include <cdohelpers.h>

#include <QDebug>
#include <QTime>

#include <sstream>
#include <time.h>


AppController::AppController(QObject *parent) :
    QObject(parent), _connected(false)
{
    QObject::connect(&_cdoCtrl,
                     SIGNAL(audioCaptureDeviceListChanged(QVariantMap)),
                     this, SLOT(onAudioCaptureDevices(QVariantMap)));
    QObject::connect(&_cdoCtrl,
                     SIGNAL(audioOutputDeviceListChanged(QVariantMap)),
                     this, SLOT(onAudioOutputDevices(QVariantMap)));
    QObject::connect(&_cdoCtrl,
                     SIGNAL(videoCaptureDeviceListChanged(QVariantMap)),
                     this,  SLOT(onVideoDevices(QVariantMap)));

    // always call startLocalVideo() when video device changed for simplicity
    QObject::connect(&_cdoCtrl, SIGNAL(videoCaptureDeviceSet()),
                     &_cdoCtrl,  SLOT(startLocalVideo()));

    QObject::connect(&_cdoCtrl, SIGNAL(localPreviewStarted(QString)),
                     this,  SLOT(onLocalVideoStarted(QString)));

    QObject::connect(&_cdoCtrl, SIGNAL(platformReady(QString)),
                     this,  SLOT(onPlatformReady(QString)));
    QObject::connect(&_cdoCtrl, SIGNAL(connected(bool)),
                     this,  SLOT(onConnected(bool)));
    QObject::connect(&_cdoCtrl, SIGNAL(disconnected()),
                     this,  SLOT(onDisconnected()));
}

void AppController::initADL()
{
    qDebug() << "Initializing ADL";
    _cdoCtrl.initPlatform();
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
    descr.authDetails.salt = ADLHelpers::stdString2ADLString("Some Salt 323234#@");
    descr.authDetails.expires = time(NULL) + 300000;
    descr.scopeId = ADLHelpers::stdString2ADLString(scopeId.toStdString());

    descr.url = ADLHelpers::stdString2ADLString(
                addlive::gStreamerAddress + "/" + scopeId.toStdString());

    descr.videoStream.maxWidth = addlive::gMaxVideoWidth;
    descr.videoStream.maxHeight = addlive::gMaxVideoHeight;
    descr.videoStream.maxFps = addlive::gMaxFps;
    descr.videoStream.useAdaptation = addlive::gUseAdaptation;

    _scopeId = scopeId.toStdString();
    _cdoCtrl.connect(&descr, scopeId.toStdString());
}

void AppController::setVideoCaptureDevice(const std::string& deviceId)
{
    qDebug() << "Setting video capture device with ID " << deviceId.c_str();
    _cdoCtrl.setVideoCaptureDevice(deviceId);
}

void AppController::setAudioCaptureDevice(const std::string& deviceId)
{
    qDebug() << "Setting audio capture device with ID " << deviceId.c_str();
    _cdoCtrl.setAudioCaptureDevice(deviceId);
}

void AppController::setAudioOutputDevice(const std::string& deviceId)
{
    qDebug() << "Setting audio output device with ID " << deviceId.c_str();
    _cdoCtrl.setAudioOutputDevice(deviceId);
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
    qDebug() << "Stopping connection";
    _cdoCtrl.disconnect(_scopeId);
}


void AppController::videoPublishStateChanged(bool state)
{
    if (_connected)
    {
        if (state)
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
    if (_connected)
    {
        if (state)
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

void AppController::onPlatformReady(QString version)
{
    qDebug() << "ADL Initialized, version: " << version <<
                ". Continuing with initialization";
    emit cdoReady(_cdoCtrl.getPlatformHandler(), version);

    ADLServiceListener listener;
    memset(&listener, 0, sizeof(listener));
    listener.onUserEvent = &AppController::onUserEvent;
    listener.onMediaStreamEvent = &AppController::onMediaEvent;
    listener.opaque = this;
    _cdoCtrl.addPlatformListener(&listener);
    _cdoCtrl.getVideoCaptureDeviceNames();
    _cdoCtrl.getAudioCaptureDeviceNames();
    _cdoCtrl.getAudioOutputDeviceNames();
}

void AppController::onVideoDevices(QVariantMap devs)
{
    qDebug() << "Got video devices list containing " << devs.size() << " items";
    emit mediaDevicesListChanged(VIDEO_IN, devs);
}

void AppController::onAudioCaptureDevices(QVariantMap devs)
{
    qDebug() << "Got audio capture devices list containing " << devs.size() << " items";
    emit mediaDevicesListChanged(AUDIO_IN, devs);
}

void AppController::onAudioOutputDevices(QVariantMap devs)
{
    qDebug() << "Got audio output devices list containing " << devs.size() << " items";
    emit mediaDevicesListChanged(AUDIO_OUT, devs);
}

void AppController::onLocalVideoStarted(QString sinkId)
{
    qDebug() << "Local preview started; Sink id: " << sinkId;
    emit localVideoSinkChanged(sinkId);
}

void AppController::onConnected(bool succ)
{
    qDebug() << "Got connected result: " << succ;
    _connected = true;
    if (succ)
    {
        emit connected();
    }
}

void AppController::onDisconnected()
{
    qDebug() << "Disconnected";
    _connected = false;
    emit disconnected();
}

void AppController::onUserEvent(const ADLUserStateChangedEvent* e)
{
    qDebug() << "Got new user event";
    if(e->isConnected && e->videoPublished)
        emit remoteVideoSinkChanged(ADLHelpers::ADLString2QString(&e->videoSinkId));
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


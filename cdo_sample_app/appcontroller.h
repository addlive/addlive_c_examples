#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <cloudeoctrl.h>
#include <addlive_sdk.h>
#include <QObject>
#include <QVariant>
#include <QMap>
#include <map>
#include <string>

enum MediaDeviceType
{
    AUDIO_IN, AUDIO_OUT, VIDEO_IN
};

class AppController : public QObject
{
    Q_OBJECT
public:

    explicit AppController(QObject *parent = 0);

    void initADL();

    void connect(QString scopeId, bool pAudio, bool pVideo);

    void setVideoCaptureDevice(const std::string& deviceId);
    void setAudioCaptureDevice(const std::string& deviceId);
    void setAudioOutputDevice(const std::string& deviceId);

signals:

    void cdoReady(void* ph, QString v);
    
    void mediaDevicesListChanged(int, QVariantMap);

    void localVideoSinkChanged(QString);
    void remoteVideoSinkChanged(QString);
    void connected();
    void disconnected();

public slots:

    void onPlatformReady(QString);

    void disconnectBtnClicked();
    void playTestSndClicked();
    void videoPublishStateChanged(bool state);
    void audioPublishStateChanged(bool state);

    void onVideoDevices(QVariantMap devs);
    void onAudioCaptureDevices(QVariantMap devs);
    void onAudioOutputDevices(QVariantMap devs);

    void onLocalVideoStarted(QString);
    void onConnected(bool succ);
    void onDisconnected();

private:

    static void onUserEvent(void* opaque, const ADLUserStateChangedEvent*);
    static void onMediaEvent(void* opaque, const ADLUserStateChangedEvent*);
    void onUserEvent(const ADLUserStateChangedEvent*);
    void onMediaEvent(const ADLUserStateChangedEvent*);


    CloudeoCtrl _cdoCtrl;
    bool _connected;
    std::string _scopeId;
};

#endif // APPCONTROLLER_H

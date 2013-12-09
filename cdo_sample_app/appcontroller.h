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
    AUDIO_IN, AUDIO_OUT, VIDEO_IN, SCREEN
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
    void remoteScreenSinkChanged(QString);
    void connected();
    void disconnected();
    void messageReceived(QString msg);
    void connectionLost(int errCode, QString errMessage);
    void sessionReconnected();

public slots:

    void onPlatformReady(QString);

    void disconnectBtnClicked();
    void playTestSndClicked();
    void videoPublishStateChanged(bool state);
    void audioPublishStateChanged(bool state);
    void screenPublishStateChanged(bool state, QString sourceId);

    void onVideoDevices(QVariantMap devs);
    void onScreenCaptureSources(QVariantMap srcs);
    void onAudioCaptureDevices(QVariantMap devs);
    void onAudioOutputDevices(QVariantMap devs);

    void onLocalVideoStarted(QString);
    void onConnected(bool succ);
    void onDisconnected();

    void sendMessageClicked();
private:

    static void onUserEvent(void* opaque, const ADLUserStateChangedEvent*);
    static void onMediaEvent(void* opaque, const ADLUserStateChangedEvent*);
    static void onMessageEvent(void* opaque, const ADLMessageEvent*);
    static void onConnectionLost(void* opaque, const ADLConnectionLostEvent*);
    static void onSessionReconnected(void* opaque, const ADLSessionReconnectedEvent*);
    void onUserEvent(const ADLUserStateChangedEvent*);
    void onMediaEvent(const ADLUserStateChangedEvent*);
    void onMessageEvent(const ADLMessageEvent*);
    void onConnectionLost(const ADLConnectionLostEvent*);
    void onSessionReconnected(const ADLSessionReconnectedEvent*);

    CloudeoCtrl _cdoCtrl;
    bool _connected;
    std::string _scopeId;
};

#endif // APPCONTROLLER_H

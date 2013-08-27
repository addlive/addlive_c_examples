#ifndef CLOUDEOCTRL_H
#define CLOUDEOCTRL_H

#include <addlive_sdk.h>

#include <QObject>
#include <QVariant>

#include <map>
#include <string>


class CloudeoCtrl: public QObject
{
    Q_OBJECT
public:

    CloudeoCtrl();

    void initPlatform();

    void addPlatformListener(ADLServiceListener* listener);

    ADLH getPlatformHandler() const;

    void getVideoCaptureDeviceNames();
    void getAudioCaptureDeviceNames();
    void getAudioOutputDeviceNames();

    void setVideoCaptureDevice(const std::string& devId);
    void setAudioCaptureDevice(const std::string& devId);
    void setAudioOutputDevice(const std::string& devId);
    void playTestSound();

    void connect(ADLConnectionDescriptor* descr, const std::string& scopeId);
    void disconnect(const std::string& scopeId);

    void publish(const std::string& scopeId, const std::string& what);
    void unpublish(const std::string& scopeId, const std::string& what);

signals:
    void platformReady(QString);

    void audioCaptureDeviceListChanged(QVariantMap);
    void audioOutputDeviceListChanged(QVariantMap);
    void videoCaptureDeviceListChanged(QVariantMap);

    void videoCaptureDeviceSet();
    void audioCaptureDeviceSet();
    void audioOutputDeviceSet();

    void localPreviewStarted(QString);

    void connected(bool);
    void disconnected();

public slots:

    void startLocalVideo();

private:

    static void onVideoCaptureDeviceSet(void* o, const ADLError* err);
    static void onAudioCaptureDeviceSet(void* o, const ADLError* err);
    static void onAudioOutputDeviceSet(void* o, const ADLError* err);

    static void onPlatformReady(void* o, const ADLError* err, ADLH h);

    static void onVersion(void* o, const ADLError* e, const ADLString* v);
    static void onAppIdSet(void* o, const ADLError* e);

    static void onAudioCaptureDevices(void* o, const ADLError* e, ADLDevice* devs,
                          size_t len);
    static void onAudioOutputDevices(void* o, const ADLError* e, ADLDevice* devs,
                          size_t len);
    static void onVideoCaptureDevices(void* o, const ADLError* e, ADLDevice* devs,
                          size_t len);

    static void onLocalPreviewStarted(void* o, const ADLError* e,
                                      const ADLString* v);
    static void onConnected(void* o, const ADLError* e);
    static void onDisconnected(void* o, const ADLError* e);

    ADLH _platformHandle;
};

#endif // CLOUDEOCTRL_H

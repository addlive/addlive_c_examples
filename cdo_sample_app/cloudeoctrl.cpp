#include <cloudeoctrl.h>
#include <cdohelpers.h>
#include <addlivesdkparams.h>
#include <sha256.h>

#include <QDebug>
#include <QCoreApplication>
#include <QDir>

#include <string>
#include <sstream>
#include <iomanip>

namespace
{
QVariantMap devsMap(ADLDevice* devs, size_t len)
{
    QVariantMap qDevs;
    for (size_t i = 0; i < len; ++i)
    {
        std::string id = ADLHelpers::ADLString2Std(&(devs[i].id));
        std::string label = ADLHelpers::ADLString2Std(&(devs[i].label));
        qDevs[QString::fromUtf8(id.c_str())] =
                QString::fromUtf8(label.c_str());
    }
    return qDevs;
}
#ifdef WIN32
QVariantMap srcsMap(ADLScreenCaptureSource* srcs, size_t len)
{
    QVariantMap qDevs;
    for (size_t i = 0; i < len; ++i)
    {
        std::string id = ADLHelpers::ADLString2Std(&(srcs[i].id));
        std::string label = ADLHelpers::ADLString2Std(&(srcs[i].title));
        qDevs[QString::fromUtf8(id.c_str())] =
                QString::fromUtf8(label.c_str());
    }
    return qDevs;
}
#endif

void nopRHandler(void*, const ADLError*)
{
}
}

CloudeoCtrl::CloudeoCtrl()
{
}

void CloudeoCtrl::initPlatform()
{
    QString appDir = QDir::cleanPath(QCoreApplication::applicationDirPath()
        + QDir::separator() + QString(addlive::gAddLiveSdkDirectoryName));

    ADLInitOptions options = {{0},{0}};
    ADLHelpers::stdString2ADLString(&options.logicLibPath, appDir.toUtf8().data());
    adl_init_platform(&CloudeoCtrl::onPlatformReady, &options, this);
}

void CloudeoCtrl::releasePlatform()
{
    adl_release_platform(_platformHandle);
}

ADLH CloudeoCtrl::getPlatformHandler() const
{
    return _platformHandle;
}

void CloudeoCtrl::addPlatformListener(ADLServiceListener* listener)
{
    adl_add_service_listener(&nopRHandler, _platformHandle, this, listener);
}

void CloudeoCtrl::getVideoCaptureDeviceNames()
{
    adl_get_video_capture_device_names(&CloudeoCtrl::onVideoCaptureDevices,
                                       _platformHandle, this);
}

#ifdef WIN32
void CloudeoCtrl::getScreenCaptureSources()
{
    adl_get_screen_capture_sources(&CloudeoCtrl::onScreenCaptureSources,
                                       _platformHandle, this, 100);
}
#endif

void CloudeoCtrl::getAudioCaptureDeviceNames()
{
    adl_get_audio_capture_device_names(&CloudeoCtrl::onAudioCaptureDevices,
                                       _platformHandle, this);
}

void CloudeoCtrl::getAudioOutputDeviceNames()
{
    adl_get_audio_output_device_names(&CloudeoCtrl::onAudioOutputDevices,
                                      _platformHandle, this);
}

void CloudeoCtrl::setVideoCaptureDevice(const std::string& dev)
{
    if (dev.empty())
        return;
    ADLString devId = ADLHelpers::stdString2ADLString(dev);
    adl_set_video_capture_device(&CloudeoCtrl::onVideoCaptureDeviceSet, _platformHandle,
                                 this, &devId);
}

void CloudeoCtrl::setAudioCaptureDevice(const std::string& dev)
{
    ADLString devId = ADLHelpers::stdString2ADLString(dev);
    adl_set_audio_capture_device(&CloudeoCtrl::onAudioCaptureDeviceSet, _platformHandle,
                                 this, &devId);
}

void CloudeoCtrl::setAudioOutputDevice(const std::string& dev)
{
    ADLString devId = ADLHelpers::stdString2ADLString(dev);
    adl_set_audio_output_device(&CloudeoCtrl::onAudioOutputDeviceSet, _platformHandle,
                                 this, &devId);
}

void CloudeoCtrl::playTestSound()
{
    adl_start_playing_test_sound(&nopRHandler, _platformHandle, this);
}

void CloudeoCtrl::startLocalVideo()
{
    adl_start_local_video(&CloudeoCtrl::onLocalPreviewStarted, _platformHandle,
                          this);
}


void CloudeoCtrl::connect(ADLConnectionDescriptor* descr, const std::string& scopeId)
{
    std::stringstream signatureRawBuilder;
    signatureRawBuilder << addlive::gAppId << scopeId <<
        descr->authDetails.userId <<
        ADLHelpers::ADLString2Std(&(descr->authDetails.salt)) <<
        descr->authDetails.expires << addlive::gAddLiveKey;

    std::string toHash = signatureRawBuilder.str();

    BYTE buf[SHA256_BLOCK_SIZE];
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, (const BYTE*)toHash.c_str(), toHash.size());
    sha256_final(&ctx, buf);

    std::stringstream signatureStream;
    signatureStream << std::setfill('0') << std::hex;
    for (size_t i = 0; i < SHA256_BLOCK_SIZE; ++i)
        signatureStream << std::setw(2) << (int)buf[i];
    std::string nsig = signatureStream.str();
    qDebug() << "Got signature: " << QString::fromStdString(nsig);

    descr->authDetails.signature = ADLHelpers::stdString2ADLString(nsig);
    adl_connect(&CloudeoCtrl::onConnected, _platformHandle, this, descr);
}

void CloudeoCtrl::disconnect(const std::string& scopeId)
{
    ADLString cdoScopeId = ADLHelpers::stdString2ADLString(scopeId);
    adl_disconnect(&CloudeoCtrl::onDisconnected, _platformHandle, this,
                   &cdoScopeId);
}


void CloudeoCtrl::publish(const std::string& scopeId, const std::string& what, ADLMediaPublishOptions* opts)
{
    ADLString cdoScopeId = ADLHelpers::stdString2ADLString(scopeId);
    ADLString cdoWhat = ADLHelpers::stdString2ADLString(what);
    adl_publish(&nopRHandler, _platformHandle, 0,
                &cdoScopeId, &cdoWhat, opts);
}

void CloudeoCtrl::unpublish(const std::string& scopeId, const std::string& what)
{
    ADLString cdoScopeId = ADLHelpers::stdString2ADLString(scopeId);
    ADLString cdoWhat = ADLHelpers::stdString2ADLString(what);
    adl_unpublish(&nopRHandler, _platformHandle, 0,
                &cdoScopeId, &cdoWhat);
}

void CloudeoCtrl::onPlatformReady(void* o, const ADLError* err, ADLH h)
{
    CloudeoCtrl* ctrl = (CloudeoCtrl*)o;
    if (adl_no_error(err))
    {
        ctrl->_platformHandle = h;
        adl_set_application_id(&CloudeoCtrl::onAppIdSet, h, ctrl, addlive::gAppId);
        adl_get_version(&CloudeoCtrl::onVersion, h, ctrl);
    }
    else
    {
        QString errorStr =
                QString::fromUtf8(err->err_message.body,
                                   err->err_message.length);
        qWarning() << "Failed to initialize the Cloudeo platform, due to: "
                 << errorStr;
    }
}

void CloudeoCtrl::onVersion(void* o, const ADLError*, const ADLString* v)
{
    CloudeoCtrl* ctrl = (CloudeoCtrl*) o;
    try
    {
        // emit signal
        ctrl->platformReady(QString::fromAscii(v->body, v->length));
    }
    catch(const std::exception& exception)
    {
        qDebug() << "Got an exception: " << exception.what();
    }
}

void CloudeoCtrl::onAudioCaptureDevices(void* o, const ADLError*,
                            ADLDevice* devs, size_t len)
{
    CloudeoCtrl* self = (CloudeoCtrl*)o;
    // emitting signal
    self->audioCaptureDeviceListChanged(devsMap(devs, len));
}

void CloudeoCtrl::onAudioOutputDevices(void* o, const ADLError*,
                            ADLDevice* devs, size_t len)
{
    CloudeoCtrl* self = (CloudeoCtrl*)o;
    // emitting signal
    self->audioOutputDeviceListChanged(devsMap(devs, len));
}

void CloudeoCtrl::onVideoCaptureDevices(void* o, const ADLError*,
                            ADLDevice* devs, size_t len)
{
    CloudeoCtrl* self = (CloudeoCtrl*)o;
    // emitting signal
    self->videoCaptureDeviceListChanged(devsMap(devs, len));
}

#ifdef WIN32
void CloudeoCtrl::onScreenCaptureSources(void* o, const ADLError*,
                            ADLScreenCaptureSource* devs, size_t len)
{
    CloudeoCtrl* self = (CloudeoCtrl*)o;
    // emitting signal
    self->screenCaptureSourceListChanged(srcsMap(devs, len));
}
#endif

void CloudeoCtrl::onVideoCaptureDeviceSet(void* o, const ADLError* err)
{
    if (err->err_code != 0)
    {
        qDebug() << "Failed to set device, error: " << err->err_code << "; " << err->err_message.body;
    }
    else
    {
        CloudeoCtrl* self = (CloudeoCtrl*)o;
        self->videoCaptureDeviceSet();
    }
}

void CloudeoCtrl::onAudioCaptureDeviceSet(void* o, const ADLError* err)
{
    if (err->err_code != 0)
    {
        qDebug() << "Failed to set device, error: " << err->err_code << "; " << err->err_message.body;
    }
    else
    {
        CloudeoCtrl* self = (CloudeoCtrl*)o;
        self->audioCaptureDeviceSet();
    }
}

void CloudeoCtrl::onAudioOutputDeviceSet(void* o, const ADLError* err)
{
    if (err->err_code != 0)
    {
        qDebug() << "Failed to set device, error: " << err->err_code << "; " << err->err_message.body;
    }
    else
    {
        CloudeoCtrl* self = (CloudeoCtrl*)o;
        self->audioOutputDeviceSet();
    }
}

void CloudeoCtrl::onLocalPreviewStarted(void* o, const ADLError*,
                                        const ADLString* v)
{
    qDebug() << "CloudeoCtrl::onLocalPreviewStarted";
    CloudeoCtrl* self = (CloudeoCtrl*)o;
    self->localPreviewStarted(ADLHelpers::ADLString2QString(v));
}

void CloudeoCtrl::onConnected(void* o, const ADLError* e)
{
    if (!adl_no_error(e))
    {
        qWarning() << "Failed to connect due to: " <<
                      ADLHelpers::ADLString2QString(&(e->err_message));
    }
    qDebug() << "Connected!";
    CloudeoCtrl* self = (CloudeoCtrl*)o;
    // emit signal
    self->connected(adl_no_error(e));
}

void CloudeoCtrl::onDisconnected(void* o, const ADLError*)
{
    CloudeoCtrl* self = (CloudeoCtrl*)o;
    // emit signal
    self->disconnected();
}

void CloudeoCtrl::onAppIdSet(void*, const ADLError*)
{
    qDebug() << "Application ID set";
}

void CloudeoCtrl::sendMessage(const std::string& scopeId)
{
    ADLString cdoScopeId = ADLHelpers::stdString2ADLString(scopeId);
    char message[512] = "Test message";
    adl_send_message(&nopRHandler, _platformHandle, this, &cdoScopeId, message, strlen(message), NULL);
}

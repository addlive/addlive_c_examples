#include <cloudeoctrl.h>
#include <cdohelpers.h>
#include <addlivesdkparams.h>

#include <cryptlite/sha256.h>
#include <cryptlite/hmac.h>

#include <QDebug>
#include <QCoreApplication>
#include <QDir>

#include <string>
#include <sstream>


CloudeoCtrl::CloudeoCtrl()
{
}

void CloudeoCtrl::initPlatform(ADLReadyHandler readyHandler)
{
    QString appDir = QDir::cleanPath(QCoreApplication::applicationDirPath()
        + QDir::separator() + QString(addlive::gAddLiveSdkDirectoryName));

    ADLInitOptions options = {{0},{0}};
    ADLHelpers::stdString2ADLString(&options.logicLibPath, appDir.toUtf8().data());
    _readyHandler = readyHandler;
    adl_init_platform(&CloudeoCtrl::onPlatformReady, &options, this);
}

void nopRHandler(void*, const ADLError*)
{
}

void CloudeoCtrl::addPlatformListener(ADLServiceListener* listener)
{
    adl_add_service_listener(&nopRHandler,_platformHandle, this, listener);
}

void CloudeoCtrl::getVideoCaptureDeviceNames(ADLDevsHandler resultHandler)
{
    ADLDevsHandler * copy = new ADLDevsHandler();
    *copy = resultHandler;
    adl_get_video_capture_device_names(&CloudeoCtrl::onDevices,_platformHandle,
                                       copy);
}

void CloudeoCtrl::getAudioCaptureDeviceNames(ADLDevsHandler  resultHandler)
{
    ADLDevsHandler * copy = new ADLDevsHandler();
    *copy = resultHandler;
    adl_get_audio_capture_device_names(&CloudeoCtrl::onDevices,_platformHandle,
                                       copy);
}

void CloudeoCtrl::getAudioOutputDeviceNames(ADLDevsHandler resultHandler)
{
    ADLDevsHandler * copy = new ADLDevsHandler();
    *copy = resultHandler;
    adl_get_audio_output_device_names(&CloudeoCtrl::onDevices,_platformHandle,
                                       copy);
}

void CloudeoCtrl::setVideoCaptureDevice(ADLSetDevHandler rH, std::string dev)
{
    ADLString devId = ADLHelpers::stdString2ADLString(dev);
    ADLSetDevHandler * copy = new ADLSetDevHandler();
    *copy = rH;

    adl_set_video_capture_device(&CloudeoCtrl::onSetDevice, _platformHandle,
                                 copy, &devId);
}

void CloudeoCtrl::setAudioCaptureDevice(ADLSetDevHandler rH,std::string dev)
{
    ADLString devId = ADLHelpers::stdString2ADLString(dev);
    ADLSetDevHandler * copy = new ADLSetDevHandler();
    *copy = rH;
    adl_set_audio_capture_device(&CloudeoCtrl::onSetDevice, _platformHandle,
                                 copy, &devId);
}

void CloudeoCtrl::setAudioOutputDevice(ADLSetDevHandler rH, std::string dev)
{
    ADLString devId = ADLHelpers::stdString2ADLString(dev);
    ADLSetDevHandler * copy = new ADLSetDevHandler();
    *copy = rH;
    adl_set_audio_output_device(&CloudeoCtrl::onSetDevice, _platformHandle,
                                 copy, &devId);
}

void CloudeoCtrl::playTestSound()
{
    adl_start_playing_test_sound(&CloudeoCtrl::nopRHandler, _platformHandle,
                                 this);
}

void CloudeoCtrl::startLocalVideo(ADLLocalVideoStartedHandler rH)
{
    ADLLocalVideoStartedHandler * copy = new ADLLocalVideoStartedHandler();
    *copy = rH;
    adl_start_local_video(&CloudeoCtrl::onLocalPreviewStarted, _platformHandle,
                          copy);
}


void CloudeoCtrl::connect(ADLConnectedHandler rH,
                          ADLConnectionDescriptor* descr, std::string scopeId)
{
    ADLConnectedHandler* copy = new ADLConnectedHandler();
    *copy = rH;

    std::stringstream signatureRawBuilder;
    signatureRawBuilder << addlive::gAppId << scopeId <<
        descr->authDetails.userId <<
        ADLHelpers::ADLString2Std(&(descr->authDetails.salt)) <<
        descr->authDetails.expires << addlive::gAddLiveKey;

    using namespace cryptlite;
    std::string signature = sha256::hash_hex(signatureRawBuilder.str());
    qDebug() << "Got signature: " << QString::fromStdString(signature);
    descr->authDetails.signature = ADLHelpers::stdString2ADLString(signature);
    adl_connect(&CloudeoCtrl::onConnected, _platformHandle, copy, descr);
}

void CloudeoCtrl::disconnect(std::string scopeId)
{
    ADLString cdoScopeId = ADLHelpers::stdString2ADLString(scopeId);
    adl_disconnect(&CloudeoCtrl::nopRHandler, _platformHandle, this,
                   &cdoScopeId);
}


void CloudeoCtrl::publish(std::string scopeId, std::string what)
{
    ADLString cdoScopeId = ADLHelpers::stdString2ADLString(scopeId);
    ADLString cdoWhat = ADLHelpers::stdString2ADLString(what);
    adl_publish(&CloudeoCtrl::nopRHandler,_platformHandle,0,
                &cdoScopeId, &cdoWhat, NULL);
}

void CloudeoCtrl::unpublish(std::string scopeId, std::string what)
{
    ADLString cdoScopeId = ADLHelpers::stdString2ADLString(scopeId);
    ADLString cdoWhat = ADLHelpers::stdString2ADLString(what);
    adl_unpublish(&CloudeoCtrl::nopRHandler,_platformHandle,0,
                &cdoScopeId, &cdoWhat);
}

void CloudeoCtrl::onPlatformReady(void* o, const ADLError* err, ADLH h)
{
    CloudeoCtrl* ctrl = (CloudeoCtrl*)o;
    if(adl_no_error(err))
    {
        ctrl->_platformHandle = h;
        adl_set_application_id(&CloudeoCtrl::onAppIdSet, h, ctrl, addlive::gAppId);
        adl_get_version(&CloudeoCtrl::onVersion,h, ctrl);
    }
    else
    {
        QString errorStr =
                QString::fromAscii(err->err_message.body,
                                   err->err_message.length );
        qWarning() << "Failed to initialize the Cloudeo platform, due to: "
                 << errorStr;
    }
}

void CloudeoCtrl::onVersion(void* o, const ADLError*, const ADLString* v)
{
    CloudeoCtrl* ctrl = (CloudeoCtrl*) o;
    try
    {
        ctrl->_readyHandler(ctrl->_platformHandle, std::string(v->body, v->length));
    } catch(const std::exception& exception)
    {
        qDebug() << "Got an exception: " << exception.what();
    }
}


void CloudeoCtrl::onDevices(void* o, const ADLError*,
                            ADLDevice* devs, size_t len)
{
    ADLDevsHandler* rH = (ADLDevsHandler*) o;
    std::map<std::string,std::string> result;
    for(unsigned i=0; i < len; i++)
    {
        std::string id = ADLHelpers::ADLString2Std(&(devs[i].id));
        std::string label = ADLHelpers::ADLString2Std(&(devs[i].label));
        result[id] = label;
    }
    (*rH)(result);
    delete rH;
}

void CloudeoCtrl::onSetDevice(void* o, const ADLError* err)
{
    ADLSetDevHandler* rH = (ADLSetDevHandler*) o;
    if (err->err_code != 0)
    {
        qDebug() << "Failed to set device, error: " << err->err_code << "; " << err->err_message.body;
    }
    else
    {
        (*rH)();
    }
    delete rH;
}

void CloudeoCtrl::onLocalPreviewStarted(void* o, const ADLError*,
                                        const ADLString* v)
{
    qDebug() << "CloudeoCtrl::onLocalPreviewStarted";
    ADLLocalVideoStartedHandler* rH = (ADLLocalVideoStartedHandler*) o;
    (*rH)(ADLHelpers::ADLString2Std(v));
    delete rH;
}

void CloudeoCtrl::onConnected(void* o, const ADLError* e)
{
    if(e->err_code != 0)
        qWarning() << "Failed to connect due to: " <<
                      ADLHelpers::ADLString2QString(&(e->err_message));
    ADLConnectedHandler* rH = (ADLConnectedHandler*)o;
    (*rH)(adl_no_error(e));
    delete rH;

}

void CloudeoCtrl::onAppIdSet(void*, const ADLError*)
{
    qDebug() << "Application ID set";
}

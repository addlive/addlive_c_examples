#ifndef CONNECTIONPARAMS_H
#define CONNECTIONPARAMS_H


namespace addlive
{

// SDK file layout settings
const char gAddLiveSdkDirectoryName[] = "AddLive_sdk-win";

// connection parameters
const int gAppId = 1;
const std::string gAddLiveKey = "your key here";
const std::string gStreamerAddress = "192.168.1.33:7000";

// video uplink stream parameters
const int gMaxVideoWidth = 640;
const int gMaxVideoHeight = 480;
const int gMaxFps = 30;
const bool gUseAdaptation = true;

}


#endif // CONNECTIONPARAMS_H

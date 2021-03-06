#ifndef ADLSAMPLEAPPWINDOW_H
#define ADLSAMPLEAPPWINDOW_H

#include <QMainWindow>
#include "appcontroller.h"
#include <QMap>
#include <QVariant>
#include <QString>
#include <addlive_sdk.h>
namespace Ui
{
class CdoSampleAppWindow;
}

class CdoSampleAppWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit CdoSampleAppWindow(QWidget *parent = 0);
    ~CdoSampleAppWindow();

public slots:

    void onVideoDeviceSelected(int);
    void onMicSelected(int);
    void onSpeakerSelected(int);
    void onMediaDevicesListChanged(int, QVariantMap);
    void onLocalPreviewSinkChanged(QString);
    void onRemotePreviewSinkChanged(QString);
    void onRemoteScreenSinkChanged(QString);
    void onADLPlatformReady(void*, QString);
    void onScreenPublishStateChanged(bool state);
    void onConnectClicked();
    void onConnected();
    void onDisconnected();
    void onMessageReceived(QString msg);
signals:

private slots:

private:

    void setupBindings();

    Ui::CdoSampleAppWindow *ui;

    AppController _appController;
};

#endif // ADLSAMPLEAPPWINDOW_H

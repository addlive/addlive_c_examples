#include "cdosampleappwindow.h"
#include "ui_cdosampleappwindow.h"
#include <QDebug>
#include <QMessageBox>


CdoSampleAppWindow::CdoSampleAppWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CdoSampleAppWindow)
{
    ui->setupUi(this);
    setupBindings();
    _appController.initADL();
}


CdoSampleAppWindow::~CdoSampleAppWindow()
{
    delete ui;
}

void CdoSampleAppWindow::onVideoDeviceSelected(int index)
{
    QString deviceId = ui->camCombo->itemData(index).toString();
    _appController.setVideoCaptureDevice(deviceId.toStdString());
}

void CdoSampleAppWindow::onMicSelected(int index)
{
    QString deviceId = ui->micCombo->itemData(index).toString();
    _appController.setAudioCaptureDevice(deviceId.toStdString());
}

void CdoSampleAppWindow::onSpeakerSelected(int index)
{
    QString deviceId = ui->spkCombo->itemData(index).toString();
    _appController.setAudioOutputDevice(deviceId.toStdString());
}

void CdoSampleAppWindow::onMediaDevicesListChanged(int devType,
                                                   QVariantMap devs)
{
    QComboBox * targetCombo;
    switch (devType)
    {
    case AUDIO_IN:
        targetCombo = ui->micCombo;
        break;
    case AUDIO_OUT:
        targetCombo = ui->spkCombo;
        break;
    case VIDEO_IN:
        targetCombo = ui->camCombo;
        break;
    case SCREEN:
        targetCombo = ui->screenCombo;
        break;
    }

    targetCombo->clear();

    QVariantMap::iterator i;
    for (i = devs.begin(); i != devs.end(); ++i)
    {
        qDebug() << "Adding device: " << i.key();
        targetCombo->addItem(QIcon(), i.value().toString(), i.key());
    }
}

void CdoSampleAppWindow::onADLPlatformReady(void* pH, QString v)
{
    ui->versionLbl->setText(v);
    ui->localRenderer->setPlatformHandle(pH);
    ui->remoteRenderer->setPlatformHandle(pH);
    ui->connectBtn->setEnabled(true);
}

void CdoSampleAppWindow::onLocalPreviewSinkChanged(QString sinkId)
{
    qDebug() << "Rendering local sink with id: " << sinkId;
    ui->localRenderer->startRender(sinkId.toStdString());
}

void CdoSampleAppWindow::onRemotePreviewSinkChanged(QString sinkId)
{
    qDebug() << "Rendering remote sink with id: " << sinkId;
    if (!sinkId.isEmpty())
    {
        ui->remoteRenderer->startRender(sinkId.toStdString());
    }
}

void CdoSampleAppWindow::onRemoteScreenSinkChanged(QString sinkId)
{
    qDebug() << "Rendering remote screen sink with id: " << sinkId;
    if (!sinkId.isEmpty())
    {
        ui->remoteRenderer->startRender(sinkId.toStdString(), false);
    }
}

void CdoSampleAppWindow::onConnectClicked()
{
    qDebug() << "Establishing a connection";
    QString scopeId = ui->scopeIdTxt->text();
    bool publishAudio = ui->publishAudioChck->checkState() == Qt::Checked;
    bool publishVideo = ui->publishVideoChck->checkState() == Qt::Checked;
    _appController.connect(scopeId, publishAudio, publishVideo);
}

void CdoSampleAppWindow::onConnected()
{
    qDebug() << "Connection established";
    ui->connectBtn->setEnabled(false);
    ui->disconnectBtn->setEnabled(true);
    if (ui->publishScreenChck->checkState()) onScreenPublishStateChanged(true);
}

void CdoSampleAppWindow::onDisconnected()
{
    ui->connectBtn->setEnabled(true);
    ui->disconnectBtn->setEnabled(false);
}

void CdoSampleAppWindow::onScreenPublishStateChanged(bool state)
{
    QString sourceId = ui->screenCombo->itemData(ui->screenCombo->currentIndex()).toString();
     _appController.screenPublishStateChanged(state, sourceId);
}

void CdoSampleAppWindow::onMessageReceived(QString msg)
{
    QMessageBox::information(this, "Message received", msg);
}

void CdoSampleAppWindow::setupBindings()
{
    QObject::connect(ui->playTestSndBtn, SIGNAL(clicked()),
                     &_appController,  SLOT(playTestSndClicked()));
    QObject::connect(ui->disconnectBtn, SIGNAL(clicked()),
                     &_appController,  SLOT(disconnectBtnClicked()));

    QObject::connect(ui->camCombo, SIGNAL(currentIndexChanged(int)),
                     this, SLOT(onVideoDeviceSelected(int)));
    QObject::connect(ui->micCombo, SIGNAL(currentIndexChanged(int)),
                     this, SLOT(onMicSelected(int)));
    QObject::connect(ui->spkCombo, SIGNAL(currentIndexChanged(int)),
                     this, SLOT(onSpeakerSelected(int)));

    QObject::connect(&_appController, SIGNAL(cdoReady(void*, QString)),
                     this, SLOT(onADLPlatformReady(void*, QString)));

    QObject::connect(&_appController,
                     SIGNAL(mediaDevicesListChanged(int, QVariantMap)),
                     this, SLOT(onMediaDevicesListChanged(int, QVariantMap)));

    QObject::connect(&_appController, SIGNAL(connected()),
                     this, SLOT(onConnected()));
    QObject::connect(&_appController, SIGNAL(disconnected()),
                     this, SLOT(onDisconnected()));

    QObject::connect(&_appController, SIGNAL(localVideoSinkChanged(QString)),
                     this, SLOT(onLocalPreviewSinkChanged(QString)));
    QObject::connect(&_appController, SIGNAL(remoteVideoSinkChanged(QString)),
                     this, SLOT(onRemotePreviewSinkChanged(QString)));
    QObject::connect(&_appController, SIGNAL(remoteScreenSinkChanged(QString)),
                     this, SLOT(onRemoteScreenSinkChanged(QString)));

    QObject::connect(ui->connectBtn, SIGNAL(clicked()),
                     this, SLOT(onConnectClicked()));

    QObject::connect(ui->publishAudioChck, SIGNAL(clicked(bool)),
                     &_appController, SLOT(audioPublishStateChanged(bool)));
    QObject::connect(ui->publishVideoChck, SIGNAL(clicked(bool)),
                     &_appController, SLOT(videoPublishStateChanged(bool)));
    QObject::connect(ui->publishScreenChck, SIGNAL(clicked(bool)),
                     this, SLOT(onScreenPublishStateChanged(bool)));

    QObject::connect(ui->sendMessageBtn, SIGNAL(clicked()),
                     &_appController, SLOT(sendMessageClicked()));
    QObject::connect(&_appController, SIGNAL(messageReceived(QString)),
                     this, SLOT(onMessageReceived(QString)));
}

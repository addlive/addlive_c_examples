#ifndef RENDERINGWIDGET_H
#define RENDERINGWIDGET_H

#include <QMetaType>
#include <QWidget>
#include <QMutex>
#include <string>
#include <addlive_sdk.h>
#include "cdohelpers.h"

class RenderingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RenderingWidget(QWidget *parent = 0);
    
    void startRender(const std::string& sinkId, bool mirror=true);

    void stopRender();

    void setPlatformHandle(ADLH handle);

protected:

    void paintEvent(QPaintEvent *e);

signals:

    void renderStartedSignal(int rendererId);
    void renderStoppedSignal();
    void updateSignal();

public slots:
    
    void renderStartedSlot(int);
    void renderStoppedSlot();

private:

    void frameReceived(const ADLVideoFrame* frame);
    // TODO: extract start*Render* methods into a separate class
    void startRenderInternal(const std::string& sinkId, bool mirror);
    void startNativeRender(const std::string& sinkId, bool mirror);
    void startDirectRender(const std::string& sinkId, bool mirror);

    static void renderStarted(void*, const ADLError*, int);
    static void renderStopped(void*, const ADLError*);
    static void invalidateClbck(void*);

    static void onFrame(void* opaque, const ADLVideoFrame* frame);

    ADLH _platformHandle;
    std::string _deferredSinkId;
    bool _deferredMirror;
    int _rendererId;
    bool _started;
    QImage _frame;
    QMutex _frameLock;
};

#endif // RENDERINGWIDGET_H

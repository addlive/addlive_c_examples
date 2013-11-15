#ifndef RENDERINGWIDGET_H
#define RENDERINGWIDGET_H

#include <QWidget>
#include <string>
#include <addlive_sdk.h>

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

    static void renderStarted(void*, const ADLError*, int);
    static void renderStopped(void*, const ADLError*);
    static void invalidateClbck(void*);

    ADLH _platformHandle;
    std::string _deferredSinkId;
    bool _deferredMirror;
    int _rendererId;
    bool _started;
};

#endif // RENDERINGWIDGET_H

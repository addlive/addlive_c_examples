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
    
    void startRender(const std::string& sinkId);

    void stopRender();

    void setPlatformHandle(ADLH handle);

    static void invalidateClbck(void*);

protected:
    void paintEvent(QPaintEvent *e);


signals:

    void renderStartedSignal(int rendererId);
    void renderStoppedSignal();

public slots:
    
    void renderStartedSlot(int);
    void renderStoppedSlot();

private:

    static void renderStarted(void*, const ADLError*, int);
    static void renderStopped(void*, const ADLError*);

    void invalidateClbckImpl();

    ADLH _platformHandle;
    std::string _deferredSinkId;
    int _rendererId;
    bool _started;
};

#endif // RENDERINGWIDGET_H

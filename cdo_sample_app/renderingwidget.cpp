#include <renderingwidget.h>
#include <cdohelpers.h>
#include <QPaintEvent>
#include <QPaintEngine>
#include <QPainter>
#include <QDebug>
#include <iostream>
#include <cassert>

#ifdef _WIN32
#include <Windows.h>
#endif


RenderingWidget::RenderingWidget(QWidget *parent) :
    QWidget(parent), _rendererId(0), _started(false)
{
    setAttribute(Qt::WA_PaintOnScreen, true);

    QObject::connect(this, SIGNAL(renderStartedSignal(int)),
                     this,  SLOT(renderStartedSlot(int)));
    QObject::connect(this, SIGNAL(renderStoppedSignal()),
                     this,  SLOT(renderStoppedSlot()));
}


void RenderingWidget::startRender(const std::string& sinkId)
{
    if (_started)
    {
        // new renderer started as soon as old one stopped
        _deferredSinkId = sinkId;
        stopRender();
        return;
    }

    qDebug() << "Starting renderer on sink " << sinkId.c_str();

    ADLRenderRequest rRequest;
    ADLHelpers::stdString2ADLString(&rRequest.sinkId, sinkId);
    ADLHelpers::stdString2ADLString(&rRequest.filter, "fast_bilinear");
    rRequest.mirror = true; rRequest.invalidateCallback =
    &RenderingWidget::invalidateClbck; rRequest.opaque = this;
    rRequest.windowHandle = NULL;
    adl_render_sink(&RenderingWidget::renderStarted, _platformHandle, this,
                    &rRequest);
    setUpdatesEnabled(true);
}

void RenderingWidget::stopRender()
{
    if (_started)
    {
        qDebug() << "Stopping renderer";
        adl_stop_render(&RenderingWidget::renderStopped, _platformHandle, this,
                        _rendererId);
    }
}

void RenderingWidget::setPlatformHandle(ADLH handle)
{
    _platformHandle = handle;
}


void RenderingWidget::paintEvent(QPaintEvent *e)
{
    if (_started)
    {
        QPoint global = mapTo(nativeParentWidget(), QPoint(0,0));
        ADLDrawRequest req;
        req.left = global.x();
        req.right = req.left + width();
        req.top = global.y();
        req.bottom = req.top + height();
        req.rendererId = _rendererId;

        QPainter painter(this);

#ifdef Q_WS_WIN
        HDC hdc = painter.paintEngine()->getDC();
        req.windowHandle = hdc;
#elif defined(Q_WS_X11)
        // TODO: implement renderer for OSX and Linux
#endif

        adl_draw(_platformHandle, &req);
    }
    else
    {
        QRect r;
        if (e)
            r = e->rect();
        else
            r = rect();
        QPainter painter(this);
        painter.setBackgroundMode(Qt::OpaqueMode);
        painter.setBrush(Qt::SolidPattern);
        painter.setBackground(QBrush(QColor::fromRgb(120,120,120)));

        painter.drawRect(r);
        painter.drawText(QRectF(0, 0, 200, 20), QString("Waiting for a frame..."));
    }
}


void RenderingWidget::invalidateClbck(void* o)
{
    ((RenderingWidget*)o)->invalidateClbckImpl();
}

void RenderingWidget::renderStarted(void* o, const ADLError*,
                                    int rendererId)
{
    RenderingWidget* _this = (RenderingWidget*) o;
    // emitting signal to be dispatched in GUI thread
    _this->renderStartedSignal(rendererId);
}

void RenderingWidget::renderStopped(void* o , const ADLError*)
{
    RenderingWidget* _this = (RenderingWidget*) o;
    // emitting signal to be dispatched in GUI thread
    _this->renderStoppedSignal();
}

void RenderingWidget::renderStartedSlot(int rendererId)
{
    qDebug() << "Rendering started on renderer with id " << rendererId;
    _rendererId = rendererId;
    _started = true;
}

void RenderingWidget::renderStoppedSlot()
{
    qDebug() << "Rendering stopped";
    _started = false;
    if (!_deferredSinkId.empty())
    {
        startRender(_deferredSinkId);
        _deferredSinkId = "";
    }
}

void RenderingWidget::invalidateClbckImpl()
{
    this->update();
}

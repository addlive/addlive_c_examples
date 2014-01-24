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

#ifdef Q_WS_X11
#include <libyuv/convert.h>
#include <libyuv/convert_argb.h>
#include <libyuv/convert_from.h>
#include <libyuv/convert_from_argb.h>
#endif



RenderingWidget::RenderingWidget(QWidget *parent) :
    QWidget(parent), _rendererId(0), _started(false)
{
#ifdef _WIN32
    setAttribute(Qt::WA_PaintOnScreen, true);
#endif

    qRegisterMetaType<QSharedPointer<ADLVideoFrameHolder> >("QSharedPointer<ADLVideoFrameHolder>");
    QObject::connect(this, SIGNAL(renderStartedSignal(int)),
                     this, SLOT(renderStartedSlot(int)));
    QObject::connect(this, SIGNAL(renderStoppedSignal()),
                     this, SLOT(renderStoppedSlot()));
    QObject::connect(this, SIGNAL(updateSignal()),
                     this, SLOT(update()));
    QObject::connect(this, SIGNAL(frameReceivedSignal(QSharedPointer<ADLVideoFrameHolder>)),
                     this, SLOT(frameReceivedSlot(QSharedPointer<ADLVideoFrameHolder>)));
}

RenderingWidget::~RenderingWidget()
{
// Platform is already released here, so the call below may lead to crash
//    stopRender();
}

void RenderingWidget::startRender(const std::string& sinkId, bool mirror)
{
    if (_started)
    {
        // new renderer is to be started as soon as old one stopped
        _deferredSinkId = sinkId;
        _deferredMirror = mirror;
        stopRender();
        return;
    }
    startRenderInternal(sinkId, mirror);
}

void RenderingWidget::startRenderInternal(const std::string& sinkId, bool mirror)
{
    qDebug() << "Starting renderer on sink " << sinkId.c_str();

#ifdef Q_WS_WIN
    startNativeRender(sinkId, mirror);
#elif defined(Q_WS_X11)
    startDirectRender(sinkId, mirror);
#endif
}

void RenderingWidget::startNativeRender(const std::string& sinkId, bool mirror)
{
    ADLRenderRequest rRequest;
    ADLHelpers::stdString2ADLString(&rRequest.sinkId, sinkId);
    ADLHelpers::stdString2ADLString(&rRequest.filter, "fast_bilinear");
    rRequest.mirror = mirror; rRequest.invalidateCallback =
    &RenderingWidget::invalidateClbck; rRequest.opaque = this;
    rRequest.windowHandle = NULL;
    adl_render_sink(&RenderingWidget::renderStarted, _platformHandle, this,
                    &rRequest);
    setUpdatesEnabled(true);
}

void RenderingWidget::startDirectRender(const std::string& sinkId, bool mirror)
{
    ADLDirectRenderRequest rRequest;
    ADLHelpers::stdString2ADLString(&rRequest.sinkId, sinkId);
    rRequest.opaque = this;
    rRequest.frameCallback = &RenderingWidget::onFrame;
    adl_direct_render_sink(&RenderingWidget::renderStarted, _platformHandle, this,
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


#ifdef Q_WS_WIN
        QPainter painter(this);
        HDC hdc = painter.paintEngine()->getDC();
        req.windowHandle = hdc;
        adl_draw(_platformHandle, &req);
#elif defined(Q_WS_X11)
        QPainter painter(this);
        painter.drawImage(rect(), _frame);
        painter.end();
#endif
    // TODO: implement renderer for OSX

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
    ((RenderingWidget*)o)->updateSignal();
}

void RenderingWidget::onFrame(void* o, const ADLVideoFrame* frame)
{
    ((RenderingWidget*)o)->frameReceivedSignal(QSharedPointer<ADLVideoFrameHolder>(new ADLVideoFrameHolder(frame)));
}

void RenderingWidget::renderStarted(void* o, const ADLError*,
                                    int rendererId)
{
    // emitting signal to be dispatched in GUI thread
    ((RenderingWidget*)o)->renderStartedSignal(rendererId);
}

void RenderingWidget::renderStopped(void* o , const ADLError*)
{
    // emitting signal to be dispatched in GUI thread
    ((RenderingWidget*)o)->renderStoppedSignal();
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
        startRender(_deferredSinkId, _deferredMirror);
        _deferredSinkId = "";
    }
}

void RenderingWidget::frameReceivedSlot(QSharedPointer<ADLVideoFrameHolder> frameHolder)
{
    const ADLVideoFrame *frameData = &frameHolder->data();
    if (frameData->format == PIC_FORMAT_YUV422) // Actually it is YUY2
    {
        _frame = QImage(QSize(frameData->width, frameData->height), QImage::Format_ARGB32);
        libyuv::YUY2ToARGB(frameData->planes[0], frameData->strides[0], _frame.bits(),
                _frame.bytesPerLine(), frameData->width, frameData->height);
    }
    else if (frameData->format == PIC_FORMAT_YUV420)
    {
        _frame = QImage(QSize(frameData->width, frameData->height), QImage::Format_ARGB32);
        libyuv::I420ToARGB(frameData->planes[0], frameData->strides[0], frameData->planes[1], frameData->strides[1],
                frameData->planes[2], frameData->strides[2], _frame.bits(),
                _frame.bytesPerLine(), frameData->width, frameData->height);
    }
    else
    {
        qDebug() << "Unsupported frame format: " << frameData->format;
    }
    updateSignal();
}

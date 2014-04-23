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

#ifdef ADL_DIRECT_RENDERING
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

    QObject::connect(this, SIGNAL(renderStartedSignal(int)),
                     this, SLOT(renderStartedSlot(int)));
    QObject::connect(this, SIGNAL(renderStoppedSignal()),
                     this, SLOT(renderStoppedSlot()));
    QObject::connect(this, SIGNAL(updateSignal()),
                     this, SLOT(update()));
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

#ifndef ADL_DIRECT_RENDERING
    startNativeRender(sinkId, mirror);
#else
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


#ifdef ADL_DIRECT_RENDERING
        QPainter painter(this);
        {
            QMutexLocker locker(&_frameLock);
            painter.drawImage(rect(), _frame);
        }
        painter.end();
#elif defined(_WIN32)
        QPainter painter(this);
        HDC hdc = painter.paintEngine()->getDC();
        req.windowHandle = hdc;
        adl_draw(_platformHandle, &req);
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
    ((RenderingWidget*)o)->frameReceived(frame);
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

#ifdef ADL_DIRECT_RENDERING

void RenderingWidget::updateFrame(const ADLVideoFrame* frameData)
{
    QMutexLocker locker(&_frameLock);
    _frame = QImage(QSize(frameData->width, frameData->height), QImage::Format_ARGB32);
    if (frameData->format == PIC_FORMAT_YUY2)
    {
        libyuv::YUY2ToARGB(frameData->planes[0], frameData->strides[0], _frame.bits(),
            _frame.bytesPerLine(), frameData->width, frameData->height);
    }
    else if (frameData->format == PIC_FORMAT_MJPG)
    {
        libyuv::MJPGToARGB(frameData->planes[0], frameData->strides[0], _frame.bits(),
            _frame.bytesPerLine(), frameData->width, frameData->height,
            frameData->width, frameData->height);
    }
    else if (frameData->format == PIC_FORMAT_YUV422)
    {
        libyuv::I422ToARGB(frameData->planes[0], frameData->strides[0],
            frameData->planes[1], frameData->strides[1],
            frameData->planes[2], frameData->strides[2], _frame.bits(),
            _frame.bytesPerLine(), frameData->width, frameData->height);
    }
    else if (frameData->format == PIC_FORMAT_YUV420)
    {
        libyuv::I420ToARGB(frameData->planes[0], frameData->strides[0],
                frameData->planes[1], frameData->strides[1],
                frameData->planes[2], frameData->strides[2], _frame.bits(),
                _frame.bytesPerLine(), frameData->width, frameData->height);
    }
    else if (frameData->format == PIC_FORMAT_BGR24)
    {
        libyuv::RGB24ToARGB(frameData->planes[0], frameData->strides[0], _frame.bits(),
            _frame.bytesPerLine(), frameData->width, -frameData->height);

    }
    else
    {
        _frame = QImage();
        qDebug() << "Unsupported frame format: " << frameData->format;
    }
}
#endif

void RenderingWidget::frameReceived(const ADLVideoFrame* frame)
{
#ifdef ADL_DIRECT_RENDERING
    updateFrame(frame);
    updateSignal();
#endif
}

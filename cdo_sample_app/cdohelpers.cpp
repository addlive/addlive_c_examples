#include "cdohelpers.h"

ADLHelpers::ADLHelpers()
{
}


void ADLHelpers::stdString2ADLString(ADLString* target, const std::string& src)
{
    strncpy(target->body, src.c_str(), ADL_STRING_MAX_LEN);
    target->length = src.length();
}

ADLString ADLHelpers::stdString2ADLString(const std::string& src)
{
    ADLString result;
    stdString2ADLString(&result, src);
    return result;
}

std::string ADLHelpers::ADLString2Std(const ADLString* src)
{
    return std::string(src->body, src->length);
}

QString ADLHelpers::ADLString2QString(const ADLString* src)
{
    return QString::fromUtf8(src->body, src->length);
}


bool ADLHelpers::stringEq(const ADLString* a, const std::string& b)
{
    return b == ADLHelpers::ADLString2Std(a);
}

void ADLHelpers::cloneVideoFrame(const ADLVideoFrame* src, ADLVideoFrame* dst)
{
    dst->width = src->width;
    dst->height = src->height;
    dst->format = src->format;
    for (size_t i = 0; i < sizeof(ADLVideoFrame::strides) / sizeof(ADLVideoFrame::strides[0]); ++i)
    {
        dst->strides[i] = src->strides[i];
        if (src->strides[i] == 0 || src->planes[i] == NULL)
        {
            dst->planes[i] = NULL;
        }
        else
        {
            int planeSize;
            if (src->format == PIC_FORMAT_YUV420)
            {
                planeSize = src->strides[i] * src->height;
                if (i != 0) planeSize /= 2; // half-size for 2nd and 3rd planes
            }
            else
            {
                // TODO: handle other formats or throw 'not supported' error
                planeSize = src->strides[i] * src->height;
            }

            dst->planes[i] = new unsigned char[planeSize];
            std::copy(src->planes[i], src->planes[i] + planeSize, dst->planes[i]);
        }
    }
}


void ADLHelpers::freeVideoFrame(ADLVideoFrame* frame)
{
    for (size_t i = 0; i < sizeof(ADLVideoFrame::strides) / sizeof(ADLVideoFrame::strides[0]); ++i)
    {
        if (frame->planes[i] != NULL) delete[] frame->planes[i];
    }
}

ADLVideoFrameHolder::ADLVideoFrameHolder(const ADLVideoFrame* src)
{
    ADLHelpers::cloneVideoFrame(src, &_data);
}

ADLVideoFrameHolder::ADLVideoFrameHolder(const ADLVideoFrameHolder& src)
{
    ADLHelpers::cloneVideoFrame(&src._data, &_data);
}

ADLVideoFrameHolder::~ADLVideoFrameHolder()
{
    ADLHelpers::freeVideoFrame(&_data);
}

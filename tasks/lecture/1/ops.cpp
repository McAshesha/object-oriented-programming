#include "ops.h"
#include <algorithm>
#include <cmath>

Image Invert(const Image& src)
{
    if (src.empty())
    {
        return Image();
    }

    Image out = src.clone();
    const int totalComps = src.rows() * src.cols() * src.channels();
    for (int i = 0; i < totalComps; ++i)
    {
        out.at(i) = static_cast<unsigned char>(255 - out.at(i));
    }
    return out;
}

Image ToGrayscale(const Image& src)
{
    if (src.empty())
    {
        return Image();
    }

    if (src.channels() == 1)
    {
        return src.clone();
    }

    const int rows = src.rows();
    const int cols = src.cols();
    const int ch   = src.channels();

    Image gray(rows, cols, 1);
    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            const int base = (r * cols + c) * ch;
            const unsigned char R = src.at(base + 0);
            const unsigned char G = (ch > 1) ? src.at(base + 1) : R;
            const unsigned char B = (ch > 2) ? src.at(base + 2) : R;

            const float y = 0.299f * R + 0.587f * G + 0.114f * B;
            unsigned char Y = static_cast<unsigned char>(std::clamp(static_cast<int>(std::lround(y)), 0, 255));
            gray.at(r * cols + c) = Y;
        }
    }
    return gray;
}

Image ResizeNearest(const Image& src, int newWidth, int newHeight)
{
    if (src.empty() || newWidth <= 0 || newHeight <= 0)
    {
        return Image();
    }

    const int srcW = src.cols();
    const int srcH = src.rows();
    const int ch   = src.channels();

    Image dst(newHeight, newWidth, ch);

    const float scaleX = static_cast<float>(srcW) / static_cast<float>(newWidth);
    const float scaleY = static_cast<float>(srcH) / static_cast<float>(newHeight);

    for (int y = 0; y < newHeight; ++y)
    {
        int srcY = static_cast<int>(y * scaleY);
        if (srcY >= srcH) srcY = srcH - 1;

        for (int x = 0; x < newWidth; ++x)
        {
            int srcX = static_cast<int>(x * scaleX);
            if (srcX >= srcW) srcX = srcW - 1;

            const int srcBase = (srcY * srcW + srcX) * ch;
            const int dstBase = (y * newWidth + x) * ch;

            for (int k = 0; k < ch; ++k)
            {
                dst.at(dstBase + k) = src.at(srcBase + k);
            }
        }
    }

    return dst;
}

Image Crop(const Image& src, int x, int y, int w, int h)
{
    if (src.empty() || w <= 0 || h <= 0)
    {
        return Image();
    }

    int x2 = x + w;
    int y2 = y + h;

    Image view = src(Range(y, y2), Range(x, x2));
    if (view.empty())
    {
        return Image();
    }
    return view.clone();
}

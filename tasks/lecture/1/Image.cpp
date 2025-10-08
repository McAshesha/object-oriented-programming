#include "Image.h"

#include <cassert>
#include <cstring>

Image::Image()
    : controlBlock(nullptr),
      topLeftPointer(nullptr),
      rowsCount(0),
      colsCount(0),
      channelsCount(0),
      rowStepBytes(0)
{
}

Image::Image(int rows, int cols, int channels)
    : Image()
{
    create(rows, cols, channels);
}

Image::Image(int rows, int cols, int channels, unsigned char* data)
    : controlBlock(nullptr),
      topLeftPointer(nullptr),
      rowsCount(0),
      colsCount(0),
      channelsCount(0),
      rowStepBytes(0)
{
    if (rows <= 0 || cols <= 0 || channels <= 0 || data == nullptr)
    {
        return;
    }

    std::size_t totalBytes = static_cast<std::size_t>(rows) * static_cast<std::size_t>(cols) * static_cast<std::size_t>(channels);
    controlBlock = new ControlBlock{ data, totalBytes, 1, false /* owning external */ };
    topLeftPointer = data;
    rowsCount = rows;
    colsCount = cols;
    channelsCount = channels;
    rowStepBytes = static_cast<std::size_t>(cols) * static_cast<std::size_t>(channels);
}

Image::Image(const Image& other)
    : controlBlock(other.controlBlock),
      topLeftPointer(other.topLeftPointer),
      rowsCount(other.rowsCount),
      colsCount(other.colsCount),
      channelsCount(other.channelsCount),
      rowStepBytes(other.rowStepBytes)
{
    retain();
}

Image::Image(const Image& image, const Range& rowRange, const Range& colRange)
    : Image()
{
    if (image.empty())
    {
        return;
    }

    int rStart, rEnd, cStart, cEnd;
    clampRange(rowRange, image.rowsCount, rStart, rEnd);
    clampRange(colRange, image.colsCount, cStart, cEnd);

    if (rStart >= rEnd || cStart >= cEnd)
    {
        return;
    }

    controlBlock = image.controlBlock;
    retain();
    channelsCount = image.channelsCount;
    rowStepBytes = image.rowStepBytes;
    rowsCount = rEnd - rStart;
    colsCount = cEnd - cStart;

    std::size_t offset = static_cast<std::size_t>(rStart) * image.rowStepBytes
                       + static_cast<std::size_t>(cStart) * static_cast<std::size_t>(channelsCount);
    topLeftPointer = image.topLeftPointer + offset;
}

Image::~Image()
{
    releaseInternal();
}

Image& Image::operator=(const Image& other)
{
    if (this == &other)
    {
        return *this;
    }

    if (controlBlock == other.controlBlock &&
        topLeftPointer == other.topLeftPointer &&
        rowsCount == other.rowsCount &&
        colsCount == other.colsCount &&
        channelsCount == other.channelsCount &&
        rowStepBytes == other.rowStepBytes)
    {
        return *this;
    }

    releaseInternal();

    controlBlock = other.controlBlock;
    topLeftPointer = other.topLeftPointer;
    rowsCount = other.rowsCount;
    colsCount = other.colsCount;
    channelsCount = other.channelsCount;
    rowStepBytes = other.rowStepBytes;

    retain();
    return *this;
}

Image Image::operator()(const Range& rowRange, const Range& colRange) const
{
    return Image(*this, rowRange, colRange);
}

Image Image::clone() const
{
    if (empty())
    {
        return makeEmpty();
    }

    Image result(rowsCount, colsCount, channelsCount);

    const std::size_t contiguousRowBytes = static_cast<std::size_t>(colsCount) * static_cast<std::size_t>(channelsCount);
    for (int r = 0; r < rowsCount; ++r)
    {
        const unsigned char* srcRow = topLeftPointer + static_cast<std::size_t>(r) * rowStepBytes;
        unsigned char* dstRow = result.topLeftPointer + static_cast<std::size_t>(r) * result.rowStepBytes;
        std::memcpy(dstRow, srcRow, contiguousRowBytes);
    }
    return result;
}

void Image::copyTo(Image& image) const
{
    image = clone();
}

void Image::create(int rows, int cols, int channels)
{
    if (rows <= 0 || cols <= 0 || channels <= 0)
    {
        releaseInternal();
        return;
    }

    bool canReuse =
        controlBlock != nullptr &&
        controlBlock->owning &&
        rows == rowsCount &&
        cols == colsCount &&
        channels == channelsCount &&
        rowStepBytes == static_cast<std::size_t>(cols) * static_cast<std::size_t>(channels) &&
        topLeftPointer == controlBlock->basePointer;

    if (canReuse)
    {
        return;
    }

    releaseInternal();

    std::size_t totalBytes = static_cast<std::size_t>(rows) * static_cast<std::size_t>(cols) * static_cast<std::size_t>(channels);
    unsigned char* buffer = nullptr;
    try
    {
        buffer = new unsigned char[totalBytes];
    }
    catch (...)
    {
        controlBlock = nullptr;
        topLeftPointer = nullptr;
        rowsCount = colsCount = channelsCount = 0;
        rowStepBytes = 0;
        return;
    }

    controlBlock = new ControlBlock{ buffer, totalBytes, 1, true };
    topLeftPointer = buffer;
    rowsCount = rows;
    colsCount = cols;
    channelsCount = channels;
    rowStepBytes = static_cast<std::size_t>(cols) * static_cast<std::size_t>(channels);
}

bool Image::empty() const
{
    return topLeftPointer == nullptr || rowsCount <= 0 || colsCount <= 0 || channelsCount <= 0;
}

void Image::release()
{
    releaseInternal();
}

Image Image::col(int x) const
{
    if (empty() || x < 0 || x >= colsCount)
    {
        return makeEmpty();
    }
    return (*this)(Range(0, rowsCount), Range(x, x + 1));
}

Image Image::colRange(const Range& range) const
{
    return (*this)(Range(0, rowsCount), range);
}

Image Image::row(int y) const
{
    if (empty() || y < 0 || y >= rowsCount)
    {
        return makeEmpty();
    }
    return (*this)(Range(y, y + 1), Range(0, colsCount));
}

Image Image::rowRange(const Range& range) const
{
    return (*this)(range, Range(0, colsCount));
}

const unsigned char* Image::data() const
{
    return topLeftPointer;
}

unsigned char* Image::data()
{
    return topLeftPointer;
}

int Image::rows() const
{
    return rowsCount;
}

int Image::cols() const
{
    return colsCount;
}

int Image::total() const
{
    return rowsCount * colsCount;
}

int Image::channels() const
{
    return channelsCount;
}

unsigned char& Image::at(int index)
{
    assert(!empty());
    const std::size_t rowWidth = static_cast<std::size_t>(colsCount) * static_cast<std::size_t>(channelsCount);
    assert(index >= 0);
    std::size_t idx = static_cast<std::size_t>(index);
    int row = static_cast<int>(idx / rowWidth);
    std::size_t offsetInRow = idx % rowWidth;
    assert(row >= 0 && row < rowsCount);
    return *(topLeftPointer + static_cast<std::size_t>(row) * rowStepBytes + offsetInRow);
}

const unsigned char& Image::at(int index) const
{
    assert(!empty());
    const std::size_t rowWidth = static_cast<std::size_t>(colsCount) * static_cast<std::size_t>(channelsCount);
    assert(index >= 0);
    std::size_t idx = static_cast<std::size_t>(index);
    int row = static_cast<int>(idx / rowWidth);
    std::size_t offsetInRow = idx % rowWidth;
    assert(row >= 0 && row < rowsCount);
    return *(topLeftPointer + static_cast<std::size_t>(row) * rowStepBytes + offsetInRow);
}

Image Image::zeros(int rows, int cols, int channels)
{
    Image img(rows, cols, channels);
    if (!img.empty())
    {
        std::memset(img.topLeftPointer, 0, img.controlBlock->byteSize);
    }
    return img;
}

Image Image::values(int rows, int cols, int channels, unsigned char value)
{
    Image img(rows, cols, channels);
    if (!img.empty())
    {
        std::memset(img.topLeftPointer, value, img.controlBlock->byteSize);
    }
    return img;
}

std::size_t Image::countRef() const
{
    return controlBlock ? controlBlock->refCount : 0;
}

void Image::retain()
{
    if (controlBlock != nullptr)
    {
        ++controlBlock->refCount;
    }
}

void Image::releaseInternal()
{
    if (controlBlock == nullptr)
    {
        topLeftPointer = nullptr;
        rowsCount = colsCount = channelsCount = 0;
        rowStepBytes = 0;
        return;
    }

    assert(controlBlock->refCount > 0);
    --controlBlock->refCount;

    if (controlBlock->refCount == 0)
    {
        if (controlBlock->owning && controlBlock->basePointer != nullptr)
        {
            delete[] controlBlock->basePointer;
            controlBlock->basePointer = nullptr;
        }
        delete controlBlock;
    }

    controlBlock = nullptr;
    topLeftPointer = nullptr;
    rowsCount = colsCount = channelsCount = 0;
    rowStepBytes = 0;
}

void Image::clampRange(const Range& in, int maxValue, int& outStart, int& outEnd)
{
    int s = in.start();
    int e = in.end();

    if (s < 0) s = 0;
    if (e < 0) e = 0;

    if (s > maxValue) s = maxValue;
    if (e > maxValue) e = maxValue;

    if (s > e) s = e;

    outStart = s;
    outEnd = e;
}

Image Image::makeEmpty()
{
    return Image();
}

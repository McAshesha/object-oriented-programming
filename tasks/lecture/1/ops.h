#pragma once
#include "Image.h"

Image Invert(const Image& src);

Image ToGrayscale(const Image& src);

Image ResizeNearest(const Image& src, int newWidth, int newHeight);

Image Crop(const Image& src, int x, int y, int w, int h);

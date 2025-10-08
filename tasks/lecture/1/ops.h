#pragma once
#include "Image.h"

Image invert(const Image& src);

Image to_grayscale(const Image& src);

Image resize_nearest(const Image& src, int newWidth, int newHeight);

Image crop(const Image& src, int x, int y, int w, int h);

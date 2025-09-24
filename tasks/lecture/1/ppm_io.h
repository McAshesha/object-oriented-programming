#pragma once
#include <string>
#include "Image.h"

bool LoadImage(const std::string& path, Image& outImage);

bool SaveImage(const std::string& path, const Image& image);

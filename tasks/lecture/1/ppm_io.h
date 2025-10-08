#pragma once
#include <string>
#include "Image.h"

bool load_image(const std::string& path, Image& outImage);

bool save_image(const std::string& path, const Image& image);

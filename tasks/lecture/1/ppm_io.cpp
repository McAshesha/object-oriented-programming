#include "ppm_io.h"
#include <fstream>
#include <cctype>
#include <iostream>
#include <stdexcept>

namespace
{
    bool next_token(std::istream& is, std::string& token)
    {
        token.clear();

        for (;;)
        {
            int c = is.peek();
            if (c == EOF)
            {
                return false;
            }
            if (std::isspace(c))
            {
                is.get();
                continue;
            }
            if (c == '#')
            {
                is.get();
                while (true)
                {
                    int d = is.get();
                    if (d == '\n' || d == EOF)
                    {
                        break;
                    }
                }
                continue;
            }
            break;
        }

        while (true)
        {
            int c = is.peek();
            if (c == EOF || std::isspace(c) || c == '#')
            {
                break;
            }
            token.push_back(static_cast<char>(is.get()));
        }

        return !token.empty();
    }

    bool read_header(std::istream& is, std::string& magic, int& width, int& height, int& maxval)
    {
        std::string tok;

        if (!next_token(is, magic))
        {
            return false;
        }

        if (!next_token(is, tok))
        {
            return false;
        }
        width = std::stoi(tok);

        if (!next_token(is, tok))
        {
            return false;
        }
        height = std::stoi(tok);

        if (!next_token(is, tok))
        {
            return false;
        }
        maxval = std::stoi(tok);

        char sep;
        is.read(&sep, 1);
        if (!is.good())
        {
            return false;
        }

        return true;
    }

}

bool load_image(const std::string& path, Image& outImage)
{
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs)
    {
        return false;
    }

    std::string magic;
    int w = 0, h = 0, maxval = 0;
    if (!read_header(ifs, magic, w, h, maxval))
    {
        return false;
    }
    if (maxval != 255)
    {
        return false;
    }

    int channels = 0;
    if (magic == "P5")
    {
        channels = 1;
    }
    else if (magic == "P6")
    {
        channels = 3;
    }
    else
    {
        return false;
    }

    Image img(w > 0 && h > 0 ? h : 0, w > 0 ? w : 0, channels);
    if (img.empty())
    {
        return false;
    }

    const std::size_t bytes = static_cast<std::size_t>(w) * static_cast<std::size_t>(h) * static_cast<std::size_t>(channels);
    ifs.read(reinterpret_cast<char*>(img.data()), static_cast<std::streamsize>(bytes));
    if (!ifs)
    {
        return false;
    }

    outImage = std::move(img);
    return true;
}

bool save_image(const std::string& path, const Image& image)
{
    if (image.empty())
    {
        return false;
    }

    const int ch = image.channels();
    if (ch != 1 && ch != 3)
    {
        return false;
    }

    Image toWrite = image;
    const std::size_t contiguousRowBytes = static_cast<std::size_t>(image.cols()) * static_cast<std::size_t>(ch);
    toWrite = image.clone();

    std::ofstream ofs(path, std::ios::binary);
    if (!ofs)
    {
        return false;
    }

    if (ch == 1)
    {
        ofs << "P5\n" << image.cols() << " " << image.rows() << "\n255\n";
    }
    else
    {
        ofs << "P6\n" << image.cols() << " " << image.rows() << "\n255\n";
    }

    const std::size_t bytes = static_cast<std::size_t>(image.cols()) * static_cast<std::size_t>(image.rows()) * static_cast<std::size_t>(ch);
    ofs.write(reinterpret_cast<const char*>(toWrite.data()), static_cast<std::streamsize>(bytes));
    return static_cast<bool>(ofs);
}

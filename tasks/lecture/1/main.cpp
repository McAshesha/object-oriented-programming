#include <iostream>
#include <string>
#include <cstdlib>

#include "ppm_io.h"
#include "ops.h"

static void PrintUsage(const char* argv0)
{
    std::cout
        << "imgtool — минималистичная CLI-утилита на наших классах Image/Range от Ковалёва Всеволода Ярославовича\n"
        << "Форматы: PGM(P5), PPM(P6) (только эти работают в наших каналах rgb для Image)\n\n"
        << "Использование:\n"
        << "  " << argv0 << " info <input.ppm|pgm>\n"
        << "  " << argv0 << " invert <input> <output>\n"
        << "  " << argv0 << " gray <input> <output>\n"
        << "  " << argv0 << " crop <input> <x> <y> <w> <h> <output>\n"
        << "  " << argv0 << " resize <input> <newW> <newH> <output>\n\n"
        << "Примеры:\n"
        << "  " << argv0 << " info test.ppm\n"
        << "  " << argv0 << " invert test.ppm invert.ppm\n"
        << "  " << argv0 << " gray test.ppm gray.pgm\n"
        << "  " << argv0 << " crop test.ppm 100 80 256 256 crop.ppm\n"
        << "  " << argv0 << " resize test.ppm 320 240 resize.ppm\n";
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        PrintUsage(argv[0]);
        return 1;
    }

    std::string cmd = argv[1];

    if (cmd == "info")
    {
        if (argc != 3)
        {
            PrintUsage(argv[0]);
            return 1;
        }
        Image img;
        if (!LoadImage(argv[2], img))
        {
            std::cerr << "ERROR: failed to load image: " << argv[2] << "\n";
            return 2;
        }
        std::cout << "File: " << argv[2] << "\n"
                  << "Size: " << img.cols() << " x " << img.rows() << "\n"
                  << "Channels: " << img.channels() << "\n";
        return 0;
    }
    else if (cmd == "invert")
    {
        if (argc != 4)
        {
            PrintUsage(argv[0]);
            return 1;
        }
        Image img;
        if (!LoadImage(argv[2], img))
        {
            std::cerr << "ERROR: failed to load image: " << argv[2] << "\n";
            return 2;
        }
        Image out = Invert(img);
        if (!SaveImage(argv[3], out))
        {
            std::cerr << "ERROR: failed to save image: " << argv[3] << "\n";
            return 3;
        }
        return 0;
    }
    else if (cmd == "gray")
    {
        if (argc != 4)
        {
            PrintUsage(argv[0]);
            return 1;
        }
        Image img;
        if (!LoadImage(argv[2], img))
        {
            std::cerr << "ERROR: failed to load image: " << argv[2] << "\n";
            return 2;
        }
        Image out = ToGrayscale(img);
        if (!SaveImage(argv[3], out))
        {
            std::cerr << "ERROR: failed to save image: " << argv[3] << "\n";
            return 3;
        }
        return 0;
    }
    else if (cmd == "crop")
    {
        if (argc != 8)
        {
            PrintUsage(argv[0]);
            return 1;
        }
        const int x = std::atoi(argv[3]);
        const int y = std::atoi(argv[4]);
        const int w = std::atoi(argv[5]);
        const int h = std::atoi(argv[6]);

        Image img;
        if (!LoadImage(argv[2], img))
        {
            std::cerr << "ERROR: failed to load image: " << argv[2] << "\n";
            return 2;
        }
        Image out = Crop(img, x, y, w, h);
        if (out.empty())
        {
            std::cerr << "ERROR: crop produced empty image (check bounds)\n";
            return 2;
        }
        if (!SaveImage(argv[7], out))
        {
            std::cerr << "ERROR: failed to save image: " << argv[7] << "\n";
            return 3;
        }
        return 0;
    }
    else if (cmd == "resize")
    {
        if (argc != 6)
        {
            PrintUsage(argv[0]);
            return 1;
        }
        const int newW = std::atoi(argv[3]);
        const int newH = std::atoi(argv[4]);

        Image img;
        if (!LoadImage(argv[2], img))
        {
            std::cerr << "ERROR: failed to load image: " << argv[2] << "\n";
            return 2;
        }
        Image out = ResizeNearest(img, newW, newH);
        if (out.empty())
        {
            std::cerr << "ERROR: resize failed\n";
            return 2;
        }
        if (!SaveImage(argv[5], out))
        {
            std::cerr << "ERROR: failed to save image: " << argv[5] << "\n";
            return 3;
        }
        return 0;
    }
    else
    {
        PrintUsage(argv[0]);
        return 1;
    }
}

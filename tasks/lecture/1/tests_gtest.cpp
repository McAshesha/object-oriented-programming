#include <gtest/gtest.h>
#include "Image.h"
#include "Range.h"

TEST(RangeTest, Basics)
{
    Range defaultRange;
    EXPECT_TRUE(defaultRange.empty());
    EXPECT_EQ(defaultRange.size(), 0);

    Range r1(2, 5);
    EXPECT_FALSE(r1.empty());
    EXPECT_EQ(r1.size(), 3);
    EXPECT_EQ(r1.start(), 2);
    EXPECT_EQ(r1.end(), 5);

    Range bad(-1, 3);
    EXPECT_TRUE(bad.empty());

    Range all = Range::all();
    EXPECT_FALSE(all.empty());
    EXPECT_EQ(all.start(), 0);
}

TEST(ImageTest, CreateAndFill)
{
    Image a = Image::zeros(3, 4, 2);
    EXPECT_FALSE(a.empty());
    EXPECT_EQ(a.rows(), 3);
    EXPECT_EQ(a.cols(), 4);
    EXPECT_EQ(a.channels(), 2);
    EXPECT_EQ(a.total(), 12);
    EXPECT_EQ(a.countRef(), static_cast<std::size_t>(1));

    Image b = Image::values(2, 2, 3, 7);
    EXPECT_FALSE(b.empty());
    for (int i = 0; i < b.total() * b.channels(); ++i)
    {
        EXPECT_EQ(b.at(i), 7);
    }
}

TEST(ImageTest, CopyRefCountingAndWriteThrough)
{
    Image a = Image::values(4, 5, 1, 9);
    EXPECT_EQ(a.countRef(), static_cast<std::size_t>(1));

    Image b = a;
    EXPECT_EQ(a.countRef(), static_cast<std::size_t>(2));
    EXPECT_EQ(b.countRef(), static_cast<std::size_t>(2));

    Image c;
    c = b;
    EXPECT_EQ(a.countRef(), static_cast<std::size_t>(3));
    EXPECT_EQ(b.countRef(), static_cast<std::size_t>(3));
    EXPECT_EQ(c.countRef(), static_cast<std::size_t>(3));

    c.at(0) = 42;
    EXPECT_EQ(a.at(0), 42);

    b.release();
    EXPECT_EQ(a.countRef(), static_cast<std::size_t>(2));
    EXPECT_EQ(c.countRef(), static_cast<std::size_t>(2));
}

TEST(ImageTest, RoiAndCloneIndependence)
{
    const int rows = 3;
    const int cols = 4;
    const int ch   = 2;

    Image base(rows, cols, ch);
    const int comps = rows * cols * ch;
    for (int i = 0; i < comps; ++i)
    {
        base.at(i) = static_cast<unsigned char>(i);
    }

    Image roi = base(Range(1, 3), Range(1, 4));
    EXPECT_EQ(roi.rows(), 2);
    EXPECT_EQ(roi.cols(), 3);
    EXPECT_EQ(roi.channels(), ch);
    EXPECT_EQ(roi.countRef(), base.countRef());

    const unsigned char first = roi.at(0);
    const unsigned char expected = base.at((1 * cols + 1) * ch + 0);
    EXPECT_EQ(first, expected);

    Image deep = roi.clone();
    EXPECT_EQ(deep.countRef(), static_cast<std::size_t>(1));
    deep.at(0) = 255;
    EXPECT_EQ(roi.at(0), first);
}

TEST(ImageTest, RowColHelpers)
{
    Image img = Image::values(3, 3, 1, 5);

    Image c1 = img.col(1);
    EXPECT_EQ(c1.rows(), 3);
    EXPECT_EQ(c1.cols(), 1);

    Image r2 = img.row(2);
    EXPECT_EQ(r2.rows(), 1);
    EXPECT_EQ(r2.cols(), 3);

    Image outOfRange = img.col(100);
    EXPECT_TRUE(outOfRange.empty());
}

TEST(ImageTest, ExternalBufferLifetime)
{
    const int rows = 2, cols = 2, ch = 1;
    unsigned char external[rows * cols * ch] = { 10, 20, 30, 40 };

    {
        Image fromExt(rows, cols, ch, external);
        EXPECT_EQ(fromExt.countRef(), static_cast<std::size_t>(1));

        fromExt.at(0) = 77;
        EXPECT_EQ(external[0], 77);

        Image view = fromExt.row(0);
        EXPECT_EQ(view.countRef(), static_cast<std::size_t>(2));
    }

    EXPECT_EQ(external[0], 77);
}

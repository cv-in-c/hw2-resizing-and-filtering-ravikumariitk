#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"

#define TWO_PI 6.2831853

void normalizeL1(image img)
{
    float sum = 0;

    for (int channel = 0; channel < img.c; channel++)
    {
        for (int row = 0; row < img.h; row++)
        {
            for (int col = 0; col < img.w; col++)
            {
                sum += get_pixel(img, col, row, channel);
            }
        }
    }

    for (int channel = 0; channel < img.c; channel++)
    {
        for (int row = 0; row < img.h; row++)
        {
            for (int col = 0; col < img.w; col++)
            {
                set_pixel(img, col, row, channel, get_pixel(img, col, row, channel) / sum);
            }
        }
    }
}

image createBoxFilter(int size)
{
    image filter = make_image(size, size, 1);

    for (int row = 0; row < filter.h; row++)
    {
        for (int col = 0; col < filter.w; col++)
        {
            set_pixel(filter, col, row, 0, 1.0 / (size * size));
        }
    }

    return filter;
}

image applyConvolution(image input, image filter, int preserve)
{
    assert(filter.c == 1 || filter.c == input.c);

    int padding = filter.w / 2;
    int width = input.w + 2 * padding;
    int height = input.h + 2 * padding;
    image padded = make_image(width, height, input.c);

    if (preserve)
    {
        for (int channel = 0; channel < input.c; channel++)
        {
            for (int row = 0; row < height; row++)
            {
                for (int col = 0; col < width; col++)
                {
                    if (col < padding || col >= width - padding || row < padding || row >= height - padding)
                    {
                        set_pixel(padded, col, row, channel, 0);
                    }
                    else
                    {
                        set_pixel(padded, col, row, channel, get_pixel(input, col - padding, row - padding, channel));
                    }
                }
            }
        }
    }
    else
    {
        for (int channel = 0; channel < input.c; channel++)
        {
            for (int row = 0; row < height; row++)
            {
                for (int col = 0; col < width; col++)
                {
                    set_pixel(padded, col, row, channel, get_pixel(input, col - padding, row - padding, channel));
                }
            }
        }
    }

    image convolved = make_image(input.w, input.h, input.c);

    for (int channel = 0; channel < input.c; channel++)
    {
        for (int row = 0; row < input.h; row++)
        {
            for (int col = 0; col < input.w; col++)
            {
                float sum = 0;

                for (int j = -padding; j <= padding; j++)
                {
                    for (int i = -padding; i <= padding; i++)
                    {
                        float img_value = get_pixel(padded, col + padding + i, row + padding + j, channel);
                        float filter_value = get_pixel(filter, padding + i, padding + j, (filter.c == 1) ? 0 : channel);
                        sum += img_value * filter_value;
                    }
                }

                set_pixel(convolved, col, row, channel, sum);
            }
        }
    }

    free_image(padded);

    return convolved;
}

image createHighPassFilter()
{
    image filter = make_image(3, 3, 1);
    set_pixel(filter, 0, 0, 0, 0);
    set_pixel(filter, 1, 0, 0, -1);
    set_pixel(filter, 2, 0, 0, 0);
    set_pixel(filter, 0, 1, 0, -1);
    set_pixel(filter, 1, 1, 0, 4);
    set_pixel(filter, 2, 1, 0, -1);
    set_pixel(filter, 0, 2, 0, 0);
    set_pixel(filter, 1, 2, 0, -1);
    set_pixel(filter, 2, 2, 0, 0);
    return filter;
}

image createSharpenFilter()
{
    image filter = make_image(3, 3, 1);
    set_pixel(filter, 0, 0, 0, 0);
    set_pixel(filter, 1, 0, 0, -1);
    set_pixel(filter, 2, 0, 0, 0);
    set_pixel(filter, 0, 1, 0, -1);
    set_pixel(filter, 1, 1, 0, 5);
    set_pixel(filter, 2, 1, 0, -1);
    set_pixel(filter, 0, 2, 0, 0);
    set_pixel(filter, 1, 2, 0, -1);
    set_pixel(filter, 2, 2, 0, 0);
    return filter;
}

image createEmbossFilter()
{
    image filter = make_image(3, 3, 1);
    set_pixel(filter, 0, 0, 0, -2);
    set_pixel(filter, 1, 0, 0, -1);
    set_pixel(filter, 2, 0, 0, 0);
    set_pixel(filter, 0, 1, 0, -1);
    set_pixel(filter, 1, 1, 0, 1);
    set_pixel(filter, 2, 1, 0, 1);
    set_pixel(filter, 0, 2, 0, 0);
    set_pixel(filter, 1, 2, 0, 1);
    set_pixel(filter, 2, 2, 0, 2);
    return filter;
}

image createGaussianFilter(float sigma)
{
    int size = (int)ceilf(sigma * 6);
    if (size % 2 == 0)
        size++;

    image filter = make_image(size, size, 1);
    float sum = 0;

    for (int row = 0; row < filter.h; row++)
    {
        for (int col = 0; col < filter.w; col++)
        {
            float u = col - (size - 1) / 2;
            float v = row - (size - 1) / 2;
            float value = expf(-(u * u + v * v) / (2 * sigma * sigma));
            set_pixel(filter, col, row, 0, value);
            sum += value;
        }
    }

    normalizeL1(filter);

    return filter;
}

image addImages(image a, image b)
{
    assert(a.w == b.w && a.h == b.h && a.c == b.c);

    image result = make_image(a.w, a.h, a.c);

    for (int channel = 0; channel < a.c; channel++)
    {
        for (int row = 0; row < a.h; row++)
        {
            for (int col = 0; col < a.w; col++)
            {
                float val_a = get_pixel(a, col, row, channel);
                float val_b = get_pixel(b, col, row, channel);
                set_pixel(result, col, row, channel, val_a + val_b);
            }
        }
    }

    return result;
}

image subtractImages(image a, image b)
{
    assert(a.w == b.w && a.h == b.h && a.c == b.c);

    image result = make_image(a.w, a.h, a.c);

    for (int channel = 0; channel < a.c; channel++)
    {
        for (int row = 0; row < a.h; row++)
        {
            for (int col = 0; col < a.w; col++)
            {
                float val_a = get_pixel(a, col, row, channel);
                float val_b = get_pixel(b, col, row, channel);
                set_pixel(result, col, row, channel, val_a - val_b);
            }
        }
    }

    return result;
}

image createGxFilter()
{
    image filter = make_image(3, 3, 1);
    set_pixel(filter, 0, 0, 0, -1);
    set_pixel(filter, 1, 0, 0, 0);
    set_pixel(filter, 2, 0, 0, 1);
    set_pixel(filter, 0, 1, 0, -2);
    set_pixel(filter, 1, 1, 0, 0);
    set_pixel(filter, 2, 1, 0, 2);
    set_pixel(filter, 0, 2, 0, -1);
    set_pixel(filter, 1, 2, 0, 0);
    set_pixel(filter, 2, 2, 0, 1);
    return filter;
}

image createGyFilter()
{
    image filter = make_image(3, 3, 1);
    set_pixel(filter, 0, 0, 0, -1);
    set_pixel(filter, 1, 0, 0, -2);
    set_pixel(filter, 2, 0, 0, -1);
    set_pixel(filter, 0, 1, 0, 0);
    set_pixel(filter, 1, 1, 0, 0);
    set_pixel(filter, 2, 1, 0, 0);
    set_pixel(filter, 0, 2, 0, 1);
    set_pixel(filter, 1, 2, 0, 2);
    set_pixel(filter, 2, 2, 0, 1);
    return filter;
}

void normalizeFeatures(image img)
{
    float min_val = INFINITY;
    float max_val = -INFINITY;

    for (int channel = 0; channel < img.c; channel++)
    {
        for (int row = 0; row < img.h; row++)
        {
            for (int col = 0; col < img.w; col++)
            {
                float val = get_pixel(img, col, row, channel);
                if (val < min_val)
                    min_val = val;
                if (val > max_val)
                    max_val = val;
            }
        }
    }

    float range = max_val - min_val;

    if (range == 0)
        return;

    for (int channel = 0; channel < img.c; channel++)
    {
        for (int row = 0; row < img.h; row++)
        {
            for (int col = 0; col < img.w; col++)
            {
                float val = get_pixel(img, col, row, channel);
                set_pixel(img, col, row, channel, (val - min_val) / range);
            }
        }
    }
}

image *sobelFilter(image img)
{
    image *sobel = calloc(2, sizeof(image));

    image gxFilter = createGxFilter();
    image gyFilter = createGyFilter();

    sobel[0] = applyConvolution(img, gxFilter, 0);
    sobel[1] = applyConvolution(img, gyFilter, 0);

    free_image(gxFilter);
    free_image(gyFilter);

    return sobel;
}

image colorizeSobel(image img)
{
    image *sobel = sobelFilter(img);

    image magnitude = make_image(img.w, img.h, 1);
    image theta = make_image(img.w, img.h, 1);

    for (int row = 0; row < img.h; row++)
    {
        for (int col = 0; col < img.w; col++)
        {
            float gx = get_pixel(sobel[0], col, row, 0);
            float gy = get_pixel(sobel[1], col, row, 0);
            float mag = sqrtf(gx * gx + gy * gy);
            float angle = atan2f(gy, gx);
            set_pixel(magnitude, col, row, 0, mag);
            set_pixel(theta, col, row, 0, angle);
        }
    }

    normalizeFeatures(magnitude);

    image colorized = make_image(img.w, img.h, 3);

    for (int row = 0; row < img.h; row++)
    {
        for (int col = 0; col < img.w; col++)
        {
            float mag = get_pixel(magnitude, col, row, 0);
            float angle = get_pixel(theta, col, row, 0);
            set_pixel(colorized, col, row, 0, angle / TWO_PI);
            set_pixel(colorized, col, row, 1, mag);
            set_pixel(colorized, col, row, 2, mag);
        }
    }

    hsv_to_rgb(colorized);

    free_image(*sobel);
    free_image(*(sobel + 1));
    free(sobel);

    free_image(magnitude);
    free_image(theta);

    return colorized;
}

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include "image.h"

float get_pixel(image im, int x, int y, int c)
{
    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
    if (c < 0)
        c = 0;
    if (x >= im.h)
        x = im.h - 1;
    if (y >= im.w)
        y = im.w - 1;
    if (c >= im.c)
        c = im.c - 1;
    return im.data[x + y * (im.w) + c * (im.w) * (im.h)];
}

void set_pixel(image im, int x, int y, int c, float v)
{
    if ((x >= 0 && x < im.h) && (y >= 0 && y < im.w) && (c >= 0 && c < im.c))
        im.data[(x) + (y) * (im.w) + (c) * (im.w) * (im.h)] = v;
}

image copy_image(image im)
{
    image copy = make_image(im.w, im.h, im.c);
    copy.data = (float *)malloc((im.h) * (im.w) * (im.c) * sizeof(float));
    memcpy(copy.data, im.data, sizeof(im.data) * (im.h) * (im.w) * (im.c));
    return copy;
}

image rgb_to_grayscale(image im)
{
    assert(im.c == 3);
    image gray = make_image(im.w, im.h, 1);
    int pixel = (im.h) * (im.w);
    gray.data = (float *)malloc(pixel * sizeof(float));
    for (int i = 0; i < pixel; i++)
    {
        gray.data[i] = 0.299 * im.data[i] + 0.587 * im.data[i + (im.w) * (im.h)] + 0.114 * im.data[i + (im.w) * (im.h) * 2];
    }
    return gray;
}

void shift_image(image im, int c, float v)
{
    int pixel = (im.h) * (im.w);
    for (int i = 0; i < pixel; i++)
    {
        im.data[i + c * pixel] += v;
    }
}

void clamp_image(image im)
{
    int pixel = (im.h) * (im.w) * (im.c);
    for (int i = 0; i < pixel; i++)
    {
        if (im.data[i] < 0)
            im.data[i] = 0;
        if (im.data[i] > 1)
            im.data[i] = 1;
    }
}
// These might be handy
float three_way_max(float a, float b, float c)
{
    return (a > b) ? ((a > c) ? a : c) : ((b > c) ? b : c);
}

float three_way_min(float a, float b, float c)
{
    return (a < b) ? ((a < c) ? a : c) : ((b < c) ? b : c);
}

void rgb_to_hsv(image im)
{
    int pixel = (im.h) * (im.w);
    for (int i = 0; i < pixel; i++)
    {
        float hue, value, saturation;
        value = three_way_max(im.data[i], im.data[i + pixel], im.data[i + 2 * pixel]);
        float m = three_way_min(im.data[i], im.data[i + pixel], im.data[i + 2 * pixel]);
        float c = value - m;
        if (value == 0 && m == 0)
        {
            saturation = 0;
        }
        else
        {
            saturation = c / value;
        }
        if (c == 0)
        {
            hue = 0;
        }
        else
        {
            float h_prime = 0.0;
            float r=im.data[i];
            float g=im.data[i + pixel];
            float b=im.data[i + 2 * pixel];
            if (value == im.data[i])
            {
                h_prime = (g - b) / c;
            }
            else if (value == g)
            {
                h_prime = (b - r) / c + 2;
            }
            else if (value == b)
            {
                h_prime = (r - g) / c + 4;
            }
            if (h_prime < 0)
            {
                hue = 1 + h_prime / 6;
            }
            else
            {
                hue = h_prime / 6;
            }
        }
        im.data[i] = hue;
        im.data[i + pixel] = saturation;
        im.data[i + 2 * pixel] = value;
    }
}

void hsv_to_rgb(image im)
{
    int pixel = (im.h) * (im.w);
    for (int i = 0; i < pixel; i++)
    {
        float r, g, b;
        float hue = im.data[i];
        float saturation = im.data[i + pixel];
        float value = im.data[i + 2 * pixel];
        float c = value * saturation;
        float x = c * (1 - fabs(fmodf((hue * 6), 2) - 1));
        float m = value - c;
        if (hue >= 0 && hue < (1.0 / 6.0))
        {
            r = c;
            g = x;
            b = 0;
        }
       
        else if (hue >= (2.0 / 6.0) && hue < (3.0 / 6.0))
        {
            r = 0;
            g = c;
            b = x;
        }
         else if (hue >= (1.0 / 6.0) && hue < (2.0 / 6.0))
        {
            r = x;
            g = c;
            b = 0;
        }
        else if (hue >= (4.0 / 6.0) && hue < (5.0 / 6.0))
        {
            r = x;
            g = 0;
            b = c;
        }
        else if (hue >= (3.0 / 6.0) && hue < (4.0 / 6.0))
        {
            r = 0;
            g = x;
            b = c;
        }
        
        else
        {
            r = c;
            g = 0;
            b = x;
        }
        im.data[i] = r + m;
        im.data[i + pixel] = g + m;
        im.data[i + 2 * pixel] = b + m;
    }
}
#include "color.hpp"

// RGB -> HSV の実装
HSV RGB::ToHSV() const
{
    float r = this->r / 255.0f;
    float g = this->g / 255.0f;
    float b = this->b / 255.0f;

    float max = std::fmax(r, std::fmax(g, b));
    float min = std::fmin(r, std::fmin(g, b));
    float delta = max - min;

    float h = 0.0f, s = 0.0f, v = max;

    if (delta > 0.0f)
    {
        s = delta / max;

        if (r == max)
        {
            h = (g - b) / delta;
        }
        else if (g == max)
        {
            h = 2.0f + (b - r) / delta;
        }
        else
        {
            h = 4.0f + (r - g) / delta;
        }

        h *= 60.0f;
        if (h < 0.0f) h += 360.0f;
    }

    return HSV(h, s, v);
}

// HSV -> RGB の実装
RGB HSV::ToRGB() const 
{
    float C = v * s;
    float X = C * (1 - std::fabs(std::fmod(h / 60.0f, 2) - 1));
    float m = v - C;
    float r, g, b;

    if (0 <= h && h < 60)
    {
        r = C, g = X, b = 0;
    }
    else if (60 <= h && h < 120)
    {
        r = X, g = C, b = 0;
    }
    else if (120 <= h && h < 180)
    {
        r = 0, g = C, b = X;
    }
    else if (180 <= h && h < 240)
    {
        r = 0, g = X, b = C;
    }
    else if (240 <= h && h < 300)
    {
        r = X, g = 0, b = C;
    }
    else
    {
        r = C, g = 0, b = X;
    }

    return RGB((r + m) * 255.0f, (g + m) * 255.0f, (b + m) * 255.0f);
}
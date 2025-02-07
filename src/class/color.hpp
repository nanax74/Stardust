#pragma once

#include <cmath>

class HSV; // 前方宣言

class RGB 
{
public:
    float r, g, b; // 0.0f ~ 255.0f

    // コンストラクタ
    RGB(float r = 0, float g = 0, float b = 0) : r(r), g(g), b(b) {}

    // RGB -> HSV
    HSV ToHSV() const;
};

class HSV 
{
public:
    float h, s, v; // H: 0.0f ~ 360.0f, S: 0.0f ~ 1.0f, V: 0.0f ~ 1.0f

    // コンストラクタ
    HSV(float h = 0, float s = 0, float v = 0) : h(h), s(s), v(v) {}

    // HSV -> RGB
    RGB ToRGB() const;
};
#include "../main.hpp"
#include "GX2CopyColorBufferToScanBuffer.hpp"
#include "shaders/ColorShaderGX2.h"

uint8_t *colorVtxs;
bool colorVtxsDirty;

bool GX2CopyColorBufferToScanBuffer::isStarted = false; // 開始フラグ
float GX2CopyColorBufferToScanBuffer::hue = 0.0f;   // 色相の初期値を設定
bool GX2CopyColorBufferToScanBuffer::isSwap = false; // 画面スワップのフラグ

DECL_FUNCTION(void, GX2CopyColorBufferToScanBuffer, GX2ColorBuffer* colorBuffer, GX2ScanTarget scanTarget) // ラッパー関数の実装
{
    GX2CopyColorBufferToScanBuffer::SwapScanTarget(&scanTarget); // クラス内のbool値によって画面スワップを適用
    if(!GX2CopyColorBufferToScanBuffer::GetIsStarted()) // フラグが立っていない場合、Startを呼び出す
    {
        GX2CopyColorBufferToScanBuffer::Start(colorBuffer, scanTarget); // 最初の一回だけ呼ばれる
        GX2CopyColorBufferToScanBuffer::SetIsStarted(true);             // フラグを立てる
    }
    GX2CopyColorBufferToScanBuffer::Update(colorBuffer, scanTarget); // 繰り返し呼ばれる
    return real_GX2CopyColorBufferToScanBuffer(colorBuffer, scanTarget); // 本来呼ばれる関数を呼び出す
}

WUPS_MUST_REPLACE_FOR_PROCESS(GX2CopyColorBufferToScanBuffer, WUPS_LOADER_LIBRARY_GX2, GX2CopyColorBufferToScanBuffer, WUPS_FP_TARGET_PROCESS_ALL); // ラッパー関数と本来の関数を置き換える

void GX2CopyColorBufferToScanBuffer::Start(GX2ColorBuffer* colorBuffer, GX2ScanTarget scanTarget)
{
    // colorVtxs = (uint8_t *) MEMAllocFromMappedMemoryForGX2Ex(4 * ColorShader::cuColorAttrSize, GX2_VERTEX_BUFFER_ALIGNMENT);
    // colorVtxsDirty = true;
}

void GX2CopyColorBufferToScanBuffer::Update(GX2ColorBuffer* colorBuffer, GX2ScanTarget scanTarget)
{

    /*

    hue += 1.0f;    // 色合いを1.0fずつ増やす
    if(hue >= 360.0f) hue = 0.0f; // 360.0f以上になったら0.0fに戻す

    RGB rgb = HSV(hue, 1.0f, 1.0f).ToRGB(); //HSVをRGBに変換
    float r = rgb.r / 255;
    float g = rgb.g / 255;
    float b = rgb.b / 255;
    GX2ClearColor(colorBuffer, r, g, b, 1.0f); // 色を設定

    */

    float widthScaleFactor = 1.0f / (float)1280;
    float heightScaleFactor = 1.0f / (float)720;

    int width = 20;

    glm::vec3 positionOffsets = glm::vec3(0.0f);

    int x = 300;
    int y = 300;

    positionOffsets[0] = (x-((1280)/2)+(width/2)) * widthScaleFactor * 2.0f;
    positionOffsets[1] = -(y-((720)/2)+(width/2)) * heightScaleFactor * 2.0f;

    glm::vec3 scale(width*widthScaleFactor,width*heightScaleFactor,1.0f);

    if (colorVtxs && colorVtxsDirty)
    {
        GX2Invalidate(GX2_INVALIDATE_MODE_CPU_ATTRIBUTE_BUFFER, colorVtxs, 4 * ColorShader::cuColorAttrSize);
        colorVtxsDirty = false;
    }

    ColorShader::instance();
    // ColorShader::instance()->setShaders();
    // ColorShader::instance()->setAttributeBuffer(colorVtxs, NULL, 4);
    // ColorShader::instance()->setAngle(0);
    // ColorShader::instance()->setOffset(positionOffsets);
    // ColorShader::instance()->setScale(scale);
    // ColorShader::instance()->setColorIntensity(glm::vec4(1.0f));
    // ColorShader::instance()->draw(GX2_PRIMITIVE_MODE_QUADS, 4);
}
#include "../main.hpp"
#include "GX2CopyColorBufferToScanBuffer.hpp"

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
    real_GX2CopyColorBufferToScanBuffer(colorBuffer, scanTarget); // 本来呼ばれる関数を呼び出す
}

WUPS_MUST_REPLACE_FOR_PROCESS(GX2CopyColorBufferToScanBuffer, WUPS_LOADER_LIBRARY_GX2, GX2CopyColorBufferToScanBuffer, WUPS_FP_TARGET_PROCESS_ALL); // ラッパー関数と本来の関数を置き換える

WHBGfxShaderGroup shaderGroup; // シェーダーグループ

void GX2CopyColorBufferToScanBuffer::Start(GX2ColorBuffer* colorBuffer, GX2ScanTarget scanTarget)
{
    WHBGfxLoadGFDShaderGroup(&shaderGroup, 0, shader_gsh); // シェーダーグループをロード
    WHBGfxInitShaderAttribute(&shaderGroup, "position", 0, 0, GX2_ATTRIB_FORMAT_FLOAT_32_32_32); // 頂点バッファの設定
    WHBGfxInitShaderAttribute(&shaderGroup, "color", 1, 0, GX2_ATTRIB_FORMAT_FLOAT_32_32_32_32); // 色バッファの設定
}

void GX2CopyColorBufferToScanBuffer::Update(GX2ColorBuffer* colorBuffer, GX2ScanTarget scanTarget)
{
    hue += 1.0f;    // 色合いを1.0fずつ増やす
    if(hue >= 360.0f) hue = 0.0f; // 360.0f以上になったら0.0fに戻す

    RGB rgb = HSV(hue, 1.0f, 1.0f).ToRGB(); //HSVをRGBに変換
    float r = rgb.r / 255;
    float g = rgb.g / 255;
    float b = rgb.b / 255;
    
    // 三角形の頂点データ
    float vertices[] =
    {
        0.0f,  0.5f, 0.0f, // 上頂点
       -0.5f, -0.5f, 0.0f, // 左下頂点
        0.5f, -0.5f, 0.0f  // 右下頂点
    };

    float colors[] =
    {
        r, g, b,
        r, g, b,
        r, g, b
    };

    GX2SetVertexShader(shaderGroup.vertexShader); // シェーダーグループを設定
    GX2SetPixelShader(shaderGroup.pixelShader); // シェーダーグループを設定

    GX2Invalidate(GX2_INVALIDATE_MODE_CPU_ATTRIBUTE_BUFFER, vertices, sizeof(vertices)); // バッファを無効化
    GX2Invalidate(GX2_INVALIDATE_MODE_CPU_ATTRIBUTE_BUFFER, colors, sizeof(colors)); // バッファを無効化

    GX2SetAttribBuffer(0, sizeof(vertices), 3 * sizeof(float), vertices); // 頂点バッファを設定
    GX2SetAttribBuffer(1, sizeof(colors), 3 * sizeof(float), colors); // 色バッファを設定

    GX2SetColorBuffer(colorBuffer, GX2_RENDER_TARGET_0); // 描画先を設定
    GX2SetViewport(0.0f, 0.0f, colorBuffer->surface.width, colorBuffer->surface.height, 0.0f, 1.0f); // ビューポートを設定

    GX2DrawEx(GX2_PRIMITIVE_MODE_TRIANGLES, 3, 0, 1); // 三角形を描画
}
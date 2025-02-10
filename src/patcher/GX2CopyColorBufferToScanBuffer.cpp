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

}

void GX2CopyColorBufferToScanBuffer::Update(GX2ColorBuffer* colorBuffer, GX2ScanTarget scanTarget)
{
    hue += 1.0f;    // 色合いを1.0fずつ増やす
    if(hue >= 360.0f) hue = 0.0f; // 360.0f以上になったら0.0fに戻す

    RGB rgb = HSV(hue, 1.0f, 1.0f).ToRGB(); //HSVをRGBに変換
    float r = rgb.r / 255;
    float g = rgb.g / 255;
    float b = rgb.b / 255;
    
    GuiText guiText("Hello World!!", 24, glm::vec4(r, g, b, 1.0f));

    CVideo video(-1, -1);

    guiText.draw(&video);
}
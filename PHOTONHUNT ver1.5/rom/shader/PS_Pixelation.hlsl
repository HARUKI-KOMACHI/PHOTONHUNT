/*==============================================================================

   2D描画用ピクセルシェーダー [shader_pixel_2d.hlsl]
--------------------------------------------------------------------------------
==============================================================================*/

float4 parameter;

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

struct PS_INPUT
{
    float4 posH : SV_Position;
    float4 color : COLOR0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float hasTex : TEXCOORD1;
};
// ===== 像素化控制 =====
static const float PIXEL_SIZE = 4.0; // 像素块大小（越大越糙）
float2 PixelateUV(float2 uv, float2 texSize)
{
    float2 pixelUV = uv * texSize;
    pixelUV = floor(pixelUV / PIXEL_SIZE) * PIXEL_SIZE;
    pixelUV += PIXEL_SIZE * 0.5; // 采样块中心
    return pixelUV / texSize;
}

float4 main(PS_INPUT ps_in) : SV_Target
{
    float4 col = ps_in.color;

    if (ps_in.hasTex > 0.5f)
    {
        // ===== 获取纹理尺寸 =====
        float texW, texH;
        g_Texture.GetDimensions(texW, texH);
        float2 texSize = float2(texW, texH);

        // ===== 像素化 UV =====
        float2 uv = PixelateUV(ps_in.texcoord, texSize);

        float4 texColor = g_Texture.Sample(g_SamplerState, uv);
        col *= texColor;
    }

    return col;
    
}

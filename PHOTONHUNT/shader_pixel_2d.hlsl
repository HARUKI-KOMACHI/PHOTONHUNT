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

float4 main(PS_INPUT ps_in) : SV_Target
{
    
    float4 col = ps_in.color;

    if (ps_in.hasTex > 0.5f)
    {
        float4 texColor = g_Texture.Sample(g_SamplerState, ps_in.texcoord);
        texColor.rgb *= texColor.a;

        col *= texColor;
    }

    return col;

}

/*==============================================================================

   2D描画用ピクセルシェーダー [shader_pixel_2d.hlsl]
--------------------------------------------------------------------------------
==============================================================================*/

//struct PS_INPUT
//{
//    float4 posH : SV_POSITION; //ピクセルの座標
//    float4 color : COLOR0; //ピクセルの色
//};

//float4 main(PS_INPUT ps_in) : SV_TARGET
//{
//    return ps_in.color; //色をそのまま出力
//}



//float4 main() : SV_TARGET
//{
//    return float4(1.0f, 1.0f, 1.0f, 1.0f);
//}

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
    
    float4 texColor = g_Texture.Sample(g_SamplerState, ps_in.texcoord);
	float4 col = ps_in.color;

    //return float4(ps_in.posH.x, ps_in.posH.y, ps_in.posH.z, 1);
    //return g_Texture.Sample(g_SamplerState, ps_in.texcoord);
    return texColor;
}

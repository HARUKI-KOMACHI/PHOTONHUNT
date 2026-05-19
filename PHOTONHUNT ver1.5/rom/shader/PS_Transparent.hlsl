// ================== 物体常量（b0） ==================
cbuffer ObjectBuffer : register(b0)
{
    float4x4 world; // 世界矩阵（模型空间 → 世界空间）
};
// 和相机常量保持一致（如果 PS 也要用）
cbuffer CameraBuffer : register(b1)
{
    float4x4 view;
    float4x4 proj;
    float3 cameraPos;
    float padding;
};
// ✅ 光照数据（来自 CPU 上传）
cbuffer LightBuffer : register(b2)
{
    float3 lightDirection; // 12 bytes
    float intensity; // 4  → align to 16 ✅

    float3 lightColor; // 12 bytes
    float ambientFactor; // 4  → align to 16 ✅

    float3 lightPos; // 12 bytes  ✅新增
    float padding2; // 4  → align to 16 ✅
};

cbuffer LigthCameraBuffer : register(b3)
{
    float4x4 Ligthview;
    float4x4 Ligthproj;
    float3 LigthcameraPos;
    float Ligthpadding;
};


struct VSOutput
{
    float4 pos : SV_POSITION;
    //float2 uv : TEXCOORD0;
    
    float3 worldPos : TEXCOORD0;
    float3 worldNormal : TEXCOORD1;
    float3 cameraPos : TEXCOORD2;
    float2 uv : TEXCOORD3;
    
    float4 LightPos : TEXCOORD4;

};

Texture2D diffuseTex0 : register(t0);
SamplerState samp0 : register(s0);

Texture2D diffuseTex : register(t1);
SamplerState samp : register(s1);


// 模拟 DirectX 光栅化到屏幕坐标
float4 ClipToScreen(float4 clipPos, float width, float height)
{
    float4 ndc = clipPos / clipPos.w;
    float4 screen;
    screen.x = (ndc.x + 1.0f) * 0.5f * width;
    screen.y = (1.0f - ndc.y) * 0.5f * height; // ✅ 模拟 DX 的Y翻转
    screen.z = ndc.z;
    screen.w = 1.0f;
    return screen;
}

float4 main(VSOutput input) : SV_Target
{
        
    
// === 光照计算 ===
    float3 N = normalize(input.worldNormal);
    float3 L = normalize(-lightDirection);
    

// === 纹理取样 ===
    float2 uv = float2(input.uv.x, 1.0f - input.uv.y);
    float4 albedo = diffuseTex.Sample(samp, uv).rgba;
    //return albedo;
    
    albedo = diffuseTex.Sample(samp, uv);
    albedo.rgb *= albedo.a;
    return albedo;
}

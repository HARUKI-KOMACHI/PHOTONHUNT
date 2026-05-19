//地面网格像素shader






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

float4 main(VSOutput input) : SV_Target
{
    
    //float2 uv = input.uv;
    //float lineWidth = 0.01;

    //bool letf = uv.x <= lineWidth;
    //bool right = uv.x >= 1.0 - lineWidth;
    //bool down = uv.y <= lineWidth;
    //bool up = uv.y >= 1.0 - lineWidth;

    //bool nei = letf || right || down || up;
    //if (!nei)
    //    discard;

    //// 用 t1 作为地块颜色
    //float4 baseCol = diffuseTex.Sample(samp, uv);

    //return baseCol;
    
    
    
    float2 uv = input.uv;
    float lineWidth = 0.01;

    bool left = uv.x <= lineWidth;
    bool right = uv.x >= 1.0 - lineWidth;
    bool down = uv.y <= lineWidth;
    bool up = uv.y >= 1.0 - lineWidth;

    bool isBorder = left || right || down || up;

    // 边框：强制白色
    if (isBorder)
    {
        return float4(1.0, 1.0, 1.0, 1.0);
    }

    // 非边框：正常贴图
    return diffuseTex.Sample(samp, uv);
}

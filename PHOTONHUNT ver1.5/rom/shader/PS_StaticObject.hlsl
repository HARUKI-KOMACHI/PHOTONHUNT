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

Texture2D shadowMap : register(t0);
SamplerState shadowSamp : register(s0);
Texture2D diffuseTex : register(t1);
SamplerState samp : register(s1);


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
    float3 albedo = diffuseTex.Sample(samp, uv).rgb;
    return float4(albedo, 1.0f);
// === 使用光源视角坐标计算阴影 ===
    
    uint width, height;
    shadowMap.GetDimensions(width, height);
    
    float4 pos = ClipToScreen(input.LightPos, width, height);
    pos = input.LightPos;

    float4 shadowCoord = pos / pos.w;
    shadowCoord.xy = shadowCoord.xy * 0.5f + 0.5f; // [-1,1] → [0,1]


    shadowCoord.y = 1.0f - shadowCoord.y; // 翻转 Y
    // 超出可采样范围直接跳过
    float shadowDepth = shadowMap.Sample(shadowSamp, shadowCoord.xy).r;


    // 把当前片元也变到光源视角
    float4 lightViewPos = input.LightPos;
    float viewZ = lightViewPos.z;

    float zNear = 1.0f;
    float zFar = 200.0f;

    float currentLinear = (viewZ - zNear) / (zFar - zNear);
    currentLinear = saturate(currentLinear);

    float ndl = saturate(dot(N, L));

// 基础 bias（根据角度变化）
    float bias = lerp(0.003f, 0.0005f, ndl);

// 判断是否在阴影中
    bool inShadow = (currentLinear - bias) > shadowDepth;

// 阴影系数
    float shadowFactor = inShadow ? 0.5f : 1.0f;
    
    // 背光区域强制变暗
    if (ndl <= 0.0f)
    {
        shadowFactor = 0.5f;
    }
    //shadowFactor = 1 - currentLinear;
// === 光照合成 ===
    //float3 lighting =
    //albedo * lightColor *
    //shadowFactor;

// ==== 卡通光照分段（Toon Step）====
    float toonDiffuse =
    ndl > 0.7 ? 1.0 : // 最亮
    ndl > 0.3 ? 0.6 : // 中间亮部
                0.2; // 最暗（非阴影）

// 阴影区域强制压暗（你之前的 shadowFactor 保留）
    toonDiffuse *= shadowFactor;

// ==== 合成 =====
    float3 ambient = albedo * lightColor * ambientFactor;

    float3 lighting =
    ambient +
    albedo * lightColor * toonDiffuse;

    
    // === 描边判断（基于法线和相机方向）===
    float3 V = normalize(input.cameraPos - input.worldPos); // 视线方向
    float viewDot = dot(N, V); // 法线 vs 相机方向

// 控制描边粗细的阈值
    float edgeThreshold = 0.3f; // 0.1 = 很细   0.4 = 很粗

    //if (viewDot < edgeThreshold)
    //{
    //// 黑色描边（你可以换成你喜欢的颜色）
    //    return float4(29.0f / 255.0f, 31.0f / 255.0f, 54.0f / 255.0f, 1); // 直接黑边
    //}
   
    
    return float4(lighting, 1.0f);
}

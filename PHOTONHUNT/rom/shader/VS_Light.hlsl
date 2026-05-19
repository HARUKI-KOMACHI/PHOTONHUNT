
// ================== 物体常量（b0） ==================
cbuffer ObjectBuffer : register(b0)
{
    float4x4 world; // 世界矩阵（模型空间 → 世界空间）
};
// ================== 相机常量（b1） ==================
cbuffer CameraBuffer : register(b1)
{
    float4x4 view; // 观察矩阵（世界空间 → 相机空间）
    float4x4 proj; // 投影矩阵（相机空间 → 裁剪空间）
    float3 cameraPos; // 相机在世界坐标中的位置
    float padding; // 16字节对齐用
};
// ================== 光源常量（b2） ==================
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

// ================== 顶点输入 ==================
struct VSInput
{
    float4 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    
};

// ================== 顶点输出（交给 PS） ==================
struct VSOutput
{
    float4 pos : SV_POSITION; // 屏幕空间坐标
    float4 lightpos : TEXCOORD0;
};

// ================== 顶点着色器 ==================
VSOutput main(VSInput input)
{
    VSOutput o;

    // 世界空间坐标（行向量左乘）
    float4 worldPos = mul(world, input.pos);


    // ---------- 计算裁剪坐标 ----------
    float4 viewPos = mul(view, worldPos);
    o.pos = mul(proj, viewPos);
    
    
    o.lightpos = viewPos;
    
    return o;
}



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

cbuffer LigthCameraBuffer : register(b3)
{
    float4x4 Ligthview;
    float4x4 Ligthproj;
    float3 LigthcameraPos;
    float Ligthpadding;
};

// ================== 骨骼常量（b4） ==================
cbuffer SkinningBuffer : register(b4)
{
    float4x4 Bones[540]; // 骨骼矩阵数组（数量你可以自己定）
    int HasSkin; // 0 = 没有骨骼，1 = 有骨骼
    float3 SkinPadding; // 对齐用
};

// ================== 顶点输入 ==================
struct VSInput
{
    float4 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    //float4 color : COLOR0;
    
    // ===== 骨骼蒙皮（可选）=====
    uint4 boneIndex : BLENDINDICES; // 影响该顶点的骨骼索引
    float4 boneWeight : BLENDWEIGHT; // 对应权重
};

// ================== 顶点输出（交给 PS） ==================
struct VSOutput
{
    float4 pos : SV_POSITION; // 屏幕空间坐标
    float3 worldPos : TEXCOORD0; // 世界坐标，用于光照
    float3 worldNormal : TEXCOORD1; // 世界法线
    float3 cameraPos : TEXCOORD2; // 相机位置（直接传下去）
    float2 uv : TEXCOORD3; // 纹理坐标
    float4 LightPos : TEXCOORD4;
    //float4 color : COLOR0;
    
};

// ================== 顶点着色器 ==================
VSOutput main(VSInput input)
{
    VSOutput output;


    // 直接把模型坐标当作裁剪坐标（要求模型坐标本来就在 [-1,+1]）
    output.pos = input.pos;
    output.uv = input.uv;
    //return output;
        // ================== 骨骼蒙皮（追加） ==================
    float4 skinnedPos = input.pos;
    float3 skinnedNormal = input.normal;

    if (HasSkin != 0)
    {
        float4 posSum = float4(0, 0, 0, 0);
        float3 normalSum = float3(0, 0, 0);

        // 最多 4 根骨骼
        [unroll]
        for (int i = 0; i < 4; ++i)
        {
            float w = input.boneWeight[i];
            if (w > 0.0f)
            {
                uint idx = input.boneIndex[i];
                float4x4 boneMat = Bones[idx];
                
                // 强制所有顶点只用 Bones[0]
                //float4x4 boneMat = Bones[0];

                posSum += mul(boneMat, input.pos) * w;
                normalSum += mul((float3x3) boneMat, input.normal) * w;
                //posSum += mul(input.pos, boneMat) * w;
                //normalSum += mul(input.normal, (float3x3) boneMat) * w;
            }
        }

        skinnedPos = posSum;
        skinnedNormal = normalize(normalSum);
    }

    
     // 先转到世界空间
    //float4 worldPos = mul(world, input.pos);
    //float3 worldNormal = normalize(mul((float3x3) world, input.normal));

    float4 worldPos = mul(world, skinnedPos);
    float3 worldNormal = normalize(mul((float3x3) world, skinnedNormal));

    // 存到输出里，给光照用
    output.worldPos = worldPos.xyz;
    output.worldNormal = worldNormal;
    output.cameraPos = cameraPos;
    
    // 最后仅用于屏幕投影的 transform_camera * proj
    float4 viewPos = mul(view, worldPos);
    output.pos = mul(proj, viewPos);
    output.uv = input.uv;
   
      // 世界空间坐标（行向量左乘）
    float4 LightworldPos = mul(world, input.pos);
    // ---------- 计算裁剪坐标 ----------
    float4 LightviewPos = mul(Ligthview, LightworldPos);
    // ---------- 计算裁剪坐标 ----------
    output.LightPos = mul(Ligthproj, LightviewPos);
    
    
    //float4 LightworldPos = mul(input.pos, world);
    //float4 LightviewPos = mul(LightworldPos, view);
    //output.LightPos = mul(LightviewPos, proj);

    
    //output.LightPos = LightviewPos;
    //output.pos = mul(Ligthproj, LightviewPos);
    return output;
}


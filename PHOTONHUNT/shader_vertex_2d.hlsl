/*==============================================================================

   2D描画用頂点シェーダー [shader_vertex_2d.hlsl]
--------------------------------------------------------------------------------

==============================================================================*/

// 定数バッファ
//float4x4 mtx; //C言語から渡されたデータが入っている


// 常量缓冲区（从 C++ 传入）
cbuffer CB0 : register(b0)
{
    float4x4 mtx; // 变换矩阵
    float2 screenSize; // 屏幕尺寸（例: 1280, 720）
    float2 padding; // 对齐补充
    float2 pos;//坐标
    float2 Rotation_Scale; //旋转和缩放
}

// 输入顶点结构体
struct VS_INPUT
{
    float4 posL : POSITION0; // 顶点位置
    float4 color : COLOR0; // 顶点颜色
    float2 texcoord : TEXCOORD0; // 纹理坐标
    float3 normal : NORMAL0; // 法线（新增）
    float hasTex : TEXCOORD1; // 是否有纹理（新增）
};

// 输出顶点结构体
struct VS_OUTPUT
{
    float4 posH : SV_POSITION; // 转换后位置
    float4 color : COLOR0; // 顶点颜色
    float2 texcoord : TEXCOORD0; // 纹理坐标
    float3 normal : NORMAL0; // 法线（传递给像素着色器）
    float hasTex : TEXCOORD1; // 是否有纹理
};

VS_OUTPUT main(VS_INPUT vs_in)
{
    VS_OUTPUT vs_out;

    // ============================
    // 1. 物体自身变换
    // ============================
    float4 worldPos = mul(vs_in.posL, mtx);

    // ============================
    // 2. 相机变换（绕屏幕中心旋转、缩放 + 平移）
    // ============================

    // 缩放倍率
    float scale = Rotation_Scale.y;

    // 旋转角度（弧度）
    float rot = Rotation_Scale.x;
    float cosR = cos(rot);
    float sinR = sin(rot);

    // 屏幕中心
    float2 center = screenSize * 0.5f;

    // 平移到屏幕中心为原点
    float2 p = worldPos.xy - center;
    p = worldPos.xy;
    // 缩放
    p *= scale;

    // 旋转（逆时针）
    float2 pr;
    pr.x = p.x * cosR - p.y * sinR;
    pr.y = p.x * sinR + p.y * cosR;

    // 平移回屏幕中心 + 相机平移
    pr += /*center*/ - pos;

    // ============================
    // 3. 转换成 NDC
    // ============================

    float2 ndc;
    ndc.x = pr.x / (screenSize.x * 0.5f);
    ndc.y = pr.y / (screenSize.y * 0.5f);

    vs_out.posH = float4(ndc, 0.0f, 1.0f);

    // ============================
    // 4. 传递其他字段
    // ============================
    vs_out.color = vs_in.color;
    vs_out.texcoord = vs_in.texcoord;
    vs_out.normal = vs_in.normal;
    vs_out.hasTex = vs_in.hasTex;

    return vs_out;
}



struct VSOutput
{
    float4 pos : SV_POSITION;
    float4 lightpos : TEXCOORD0;
};

float4 main(VSOutput input) : SV_Target
{
    
    float4 lightViewPos = input.lightpos;
    float viewZ = lightViewPos.z;

    // 光源相机范围
    float zNear = 1.0f;
    float zFar = 200.0f;

    // 线性化
    float linearDepth = (viewZ - zNear) / (zFar - zNear);

    // 提高精度：用 float 而不是 saturate 截断
    linearDepth = clamp(linearDepth, 0.0f, 1.0f);

    // ⚠️ 不要乘回 255，也不要做 gamma，否则又压缩回 8bit 精度
    // 直接输出 float 值即可
    return float4(linearDepth.xxx, 1.0f); // RGB三通道全存 float32 深度
    
    // // 光源视角下的位置
    //float4 lightViewPos = input.lightpos; // 你VS里要传
    //float viewZ = lightViewPos.z; // 这是光看到的“距离”

    //// 你的光源相机的 near / far，跟主相机无关，用光源的
    //float zNear = 1.0f;
    //float zFar = 200.0f;

    //// 线性化
    //float linearDepth = (viewZ - zNear) / (zFar - zNear);

    //linearDepth = saturate(linearDepth);

    //return float4(linearDepth, linearDepth, linearDepth, 1.0f);
    
}

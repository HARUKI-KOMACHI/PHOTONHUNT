#version 330 core

layout(location = 0) in vec4 aPos;        // position (x,y,z,w)
layout(location = 1) in vec3 aNormal;     // normal
layout(location = 2) in vec2 aTexcoord;   // uv
layout(location = 3) in vec4 aColor;      // color
layout(location = 4) in float aHasTex;    // hasTexture

out vec4 vColor;
out vec2 vUV;
out float vHasTex;

uniform mat4 uProj;   // 投影矩阵（你DX里本来就有）
uniform mat4 uView;   // 视图矩阵
uniform mat4 uModel;  // 模型矩阵（当前物体的变换）




// 固定屏幕大小（你可以改成你窗口实际大小）
const vec2 screenSize = vec2(1280.0, 720.0);

// 固定矩阵 = 单位矩阵（你 DX 的 mtx 同等效果）
mat4 mtx = mat4(1.0);

void main()
{
    // 等价 DX: float4 transformed = mul(vs_in.posL, mtx);
    vec4 transformed = mtx * aPos;

    // DX 风格屏幕坐标转 NDC
    vec2 ndc;
    ndc.x = transformed.x / (screenSize.x * 0.5);
    ndc.y = transformed.y / (screenSize.y * 0.5);

    gl_Position = vec4(ndc, 0.0, 1.0);

    vColor = aColor;
    vUV = aTexcoord;
    vHasTex = aHasTex;
}

#include "OpenGL_shader.h"
#include <glad/glad.h>
#include <vector>
#include "main.h"

#include <filesystem>
#include <fstream>
GLuint VAO = 0;
GLuint VBO = 0;
GLuint IBO = 0;
GLuint shaderProgram = 0;
//VertexBuffer
bool CreateGLBuffers()
{
    //// === 创建 VAO ===
    //glGenVertexArrays(1, &VAO);
    //glBindVertexArray(VAO);

    // === 顶点数据 ===
    std::vector<Vertex_GL> vertices;

    Vertex_GL v{};
    v.color[0] = 1; v.color[1] = 1; v.color[2] = 1; v.color[3] = 1;
    v.hasTexture = 1.0f;
    v.normal[0] = 0; v.normal[1] = 0; v.normal[2] = -1;

    v.texcoord[0] = 0; v.texcoord[1] = 0;
    v.position[0] = -0.5f; v.position[1] = -0.5f; v.position[2] = 0; v.position[3] = 0;
    vertices.push_back(v);

    v.texcoord[0] = 1; v.texcoord[1] = 0;
    v.position[0] = 0.5f; v.position[1] = -0.5f;
    vertices.push_back(v);

    v.texcoord[0] = 1; v.texcoord[1] = 1;
    v.position[0] = 0.5f; v.position[1] = 0.5f;
    vertices.push_back(v);

    v.texcoord[0] = 0; v.texcoord[1] = 1;
    v.position[0] = -0.5f; v.position[1] = 0.5f;
    vertices.push_back(v);

    // === 创建 VBO ===
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex_GL), vertices.data(), GL_DYNAMIC_DRAW);

    // === 索引数据 ===
    uint16_t indices[6] = { 0,1,2, 1,2,3 };

    // === 创建 IBO ===
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    return true;
}

//IndexBuffer
void SetupVertexAttributes()
{
    // === 创建 VAO ===
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    GLsizei stride = sizeof(Vertex_GL);

    // position : location = 0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex_GL, position));

    // normal : location = 1
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex_GL, normal));

    // texcoord : location = 2
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex_GL, texcoord));

    // color : location = 3
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex_GL, color));

    // hasTexture : location = 4
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex_GL, hasTexture));

    glBindVertexArray(0);
}

//InputLayout
bool CreateGLInputLayout()
{
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    GLsizei stride = sizeof(Vertex_GL);

    // position → location = 0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 4, GL_FLOAT, GL_FALSE, stride,
        (void*)offsetof(Vertex_GL, position)
    );

    // normal → location = 1
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE, stride,
        (void*)offsetof(Vertex_GL, normal)
    );

    // texcoord → location = 2
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, 2, GL_FLOAT, GL_FALSE, stride,
        (void*)offsetof(Vertex_GL, texcoord)
    );

    // color → location = 3
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(
        3, 4, GL_FLOAT, GL_FALSE, stride,
        (void*)offsetof(Vertex_GL, color)
    );

    // hasTexture → location = 4
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(
        4, 1, GL_FLOAT, GL_FALSE, stride,
        (void*)offsetof(Vertex_GL, hasTexture)
    );

    glBindVertexArray(0);
    return true;
}

//VertexShader
//PixelShader

GLuint CompileShader(GLenum type, const char* source)
{
    //按照读进来的字符串加上ps或者vs的分类进行临时编译
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        MessageBoxA(nullptr, log, "Shader Compile Error", MB_OK);
    }
    return shader;
}


std::string LoadTextFile(const char* filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        MessageBoxA(nullptr, filename, "Shader File Not Found", MB_OK);
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}


void CreateShaderProgram()
{
    // 如果之前有 shader program，先删掉
    if (shaderProgram != 0)
    {
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }

    // 读取文本
    std::string vsText = LoadTextFile("vertex_2d.glsl");
    std::string fsText = LoadTextFile("pixel_2d.glsl");

    // 编译
    GLuint vs = CompileShader(GL_VERTEX_SHADER, vsText.c_str());
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsText.c_str());

    // 连接
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);

    // 检查错误（可选）
    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetProgramInfoLog(shaderProgram, 1024, NULL, log);
        MessageBoxA(NULL, log, "Shader Link Error", MB_OK);
    }

    // 删除 shader 对象
    glDeleteShader(vs);
    glDeleteShader(fs);

    hal::dout << "Shader Reloaded\n";

}

//ConstantBuffer//常量
    //通过直接从字符中插入数据的方式加载常量
void SetModelMatrix(GLuint program, const float* model)
{
    glUseProgram(program);

    // 获取 uniform 位置
    GLint loc = glGetUniformLocation(program, "uModel");
    if (loc == -1) {
        // uniform 在 shader 里不存在
        return;
    }

    // 上传 4x4 矩阵（float[16]）
    glUniformMatrix4fv(loc, 1, GL_FALSE, model);
}
//RasterizerState
void SetupRasterizerState_OpenGL()
{
    // ============================================================
    // 填充模式（Solid / Wireframe / Point）
    // ============================================================
    // 默认：实体
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // ★线框模式（调试用）
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // ★点渲染模式
    // glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);


    // ============================================================
    // 背面剔除（CullMode）
    // ============================================================
    // 默认：不开启剔除
    // glDisable(GL_CULL_FACE);

    // ★开启背面剔除（常规 3D 推荐）
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);        // 剔除背面

    // ★剔除正面
    // glCullFace(GL_FRONT);

    // ★剔除全部（一般不用）
    // glCullFace(GL_FRONT_AND_BACK);


    // ============================================================
    // 正面方向（FrontFace）
    // ============================================================
    // 默认逆时针为正面
    glFrontFace(GL_CCW);

    // ★如果模型是顺时针为正面
    // glFrontFace(GL_CW);


    // ============================================================
    // 多边形偏移（DepthBias / SlopeBias）
    // ============================================================
    // ★用于阴影偏移 / 避免 Z-Fighting
    // glEnable(GL_POLYGON_OFFSET_FILL);
    // glPolygonOffset(1.0f, 1.0f); // factor, units


    // ============================================================
    // 剪裁（Scissor）
    // ============================================================
    // glEnable(GL_SCISSOR_TEST);
    // glScissor(0, 0, 300, 300);


    // ============================================================
    // 多重采样（MSAA）
    // ============================================================
    glEnable(GL_MULTISAMPLE);

    // ★禁用 MSAA（可用于像素风）
    // glDisable(GL_MULTISAMPLE);


    // ============================================================
    // 线条抗锯齿（AntialiasedLineEnable）
    // ============================================================
    // glEnable(GL_LINE_SMOOTH);

    // 注意：现代 GL 一般用 MSAA，不依赖 LINE_SMOOTH
}

//DepthStencilState（深度 + 模板）完整函数
void SetupDepthStencilState_OpenGL()
{
    // ================================
    // 深度测试（Depth Test）
    // ================================
    glEnable(GL_DEPTH_TEST);                 // 启用深度测试
    glDepthFunc(GL_LESS);                    // 保留更近的像素 (默认)

    // 常用模式：
    // glDepthFunc(GL_LEQUAL);               // 阴影 / 特效更常用
    // glDepthFunc(GL_GREATER);              // 反转深度
    // glDepthFunc(GL_ALWAYS);               // 永远通过（绘制UI等）

    // 可写深度（等同 DX DepthWriteMask）
    glDepthMask(GL_TRUE);                    // 允许写入深度缓冲
    // glDepthMask(GL_FALSE);               // 不写深度（透明物体）


    // ================================
    // 模板测试（Stencil Test）
    // ================================
    // glEnable(GL_STENCIL_TEST);           // 开启模板测试

    // 完整设置（等同 DX Stencil State）
    // glStencilFunc(GL_EQUAL, 1, 0xFF);    // 比较函数
    // glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);  // 操作规则

    // glStencilMask(0xFF);                 // 模板写入 mask
}

//BlendState（混合）完整函数
void SetupBlendState_OpenGL()
{
    // 默认不开启混合
    // glDisable(GL_BLEND);

    // ================================
    // 常用透明混合（与 DX11 一样）
    // ================================
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ================================
    // 其它常用混合模式：
    // ================================

    // ★ 加法混合（特效、火、光）
    // glBlendFunc(GL_ONE, GL_ONE);

    // ★ 预乘 alpha（Premultiplied Alpha）
    // glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    // ★ 乘法混合（阴影层常用）
    // glBlendFunc(GL_DST_COLOR, GL_ZERO);

    // ★ 自定义混合（等同 DX 的 BlendDesc）
    // glBlendFuncSeparate(srcRGB, dstRGB, srcA, dstA);
}


//RenderTarget / DSV（渲染目标）完整函数
struct RenderTargetGL
{
    GLuint fbo = 0;
    GLuint colorTex = 0;
    GLuint depthTex = 0;
};

RenderTargetGL CreateRenderTargetGL(int width, int height)
{
    RenderTargetGL rt;

    // -------------------------------
    // 创建 Color Texture
    // -------------------------------
    glGenTextures(1, &rt.colorTex);
    glBindTexture(GL_TEXTURE_2D, rt.colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // -------------------------------
    // 创建 Depth Texture
    // -------------------------------
    glGenTextures(1, &rt.depthTex);
    glBindTexture(GL_TEXTURE_2D, rt.depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0,
        GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);

    // -------------------------------
    // 创建 FBO
    // -------------------------------
    glGenFramebuffers(1, &rt.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, rt.fbo);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D, rt.colorTex, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
        GL_TEXTURE_2D, rt.depthTex, 0);

    // -------------------------------
    // 检查 FBO 是否完整
    // -------------------------------
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        MessageBoxA(nullptr, "FBO 创建失败！", "Error", MB_OK);
    }

    // -------------------------------
    // 解绑 FBO
    // -------------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return rt;
}


//Viewport

//glViewport(0, 0, width, height);

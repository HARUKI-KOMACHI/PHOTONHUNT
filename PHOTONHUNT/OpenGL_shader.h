#pragma once
#include <glad/glad.h>

extern GLuint VAO;
extern GLuint VBO;
extern GLuint IBO;
extern GLuint shaderProgram;

//VertexBuffer
bool CreateGLBuffers();

//IndexBuffer
void SetupVertexAttributes();

//InputLayout
bool CreateGLInputLayout();

//VertexShader
//PixelShader

GLuint CompileShader(GLenum type, const char* source);

void CreateShaderProgram();

//ConstantBuffer//常量
    //通过直接从字符中插入数据的方式加载常量
void SetModelMatrix(GLuint program, const float* model);
//RasterizerState
void SetupRasterizerState_OpenGL();

//DepthStencilState（深度 + 模板）完整函数
void SetupDepthStencilState_OpenGL();

//BlendState（混合）完整函数
void SetupBlendState_OpenGL();


//Viewport
#pragma once
#include "ChiliTimer/ChiliTimer.h"
#include "ChiliWin.h"

class Graphics_GL
{
public:

	void depth();


public:
	int width = 0;
	int height = 0;

	HDC hDC = nullptr;         // 保存设备上下文
	HGLRC glContext = nullptr; // 保存 OpenGL 渲染上下文
public:
	Graphics_GL() {};
	Graphics_GL(HWND hWnd, int w, int h);
	Graphics_GL(const Graphics_GL&) = delete;
	Graphics_GL& operator=(const Graphics_GL&) = delete;
	~Graphics_GL() = default;


	void EndFrame();
	void ClearBuffer(float red, float green, float blue)noexcept;

	void Darw_GL(void);

	ChiliTimer timer;

public:


};


/*
// 绑定 VAO（里面包含 VBO + IBO + layout）
glBindVertexArray(vao);

// 绑定 shader
glUseProgram(shaderProgram);

// 绑定纹理
glBindTexture(GL_TEXTURE_2D, textureID);

// 设置 uniform（常量）
glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &modelMatrix[0][0]);

// 光栅化设置
glEnable(GL_CULL_FACE);
glCullFace(GL_BACK);

// 深度
glEnable(GL_DEPTH_TEST);
glDepthFunc(GL_LESS);

// 混合
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

// 绘制
glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
*/
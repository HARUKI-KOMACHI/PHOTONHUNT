#include "Graphics-OpenGL.h"


#include <glad/glad.h>
#include <windows.h>

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_opengl3.h>
#include "debug_ostream.h"
#include "OpenGL_shader.h"
#include "main.h"


Graphics_GL::Graphics_GL(HWND hWnd, int w, int h)
{

    width = w;
    height = h;

    // 1. 获取 DC
    HDC hDC = GetDC(hWnd);
    this->hDC = hDC;

    // 2. 设置像素格式
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int format = ChoosePixelFormat(hDC, &pfd);
    SetPixelFormat(hDC, format, &pfd);


    hal::dout << "PixelFormat Set: " << format << std::endl;

    // 3. 创建临时上下文
    HGLRC tempContext = wglCreateContext(hDC);

    if (tempContext)
        hal::dout << "TempContext OK\n";

    if (!tempContext)
        MessageBoxA(nullptr, "tempContext FAILED", "Error", MB_OK);

    if (!wglMakeCurrent(hDC, tempContext))
        MessageBoxA(nullptr, "wglMakeCurrent FAILED", "Error", MB_OK);

    // 4. 获取扩展函数 wglCreateContextAttribsARB
    typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int*);
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB =
        (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");


    hal::dout << "wglCreateContextAttribsARB ptr: " << (void*)wglCreateContextAttribsARB << std::endl;
    //// 5. 创建真正的 OpenGL Context (OpenGL 4.x)
   //int attribs[] =
   //{
   //    0x2091, 4, // WGL_CONTEXT_MAJOR_VERSION_ARB
   //    0x2092, 6, // WGL_CONTEXT_MINOR_VERSION_ARB
   //    0x9126, 0x00000001, // WGL_CONTEXT_PROFILE_MASK_ARB (CORE PROFILE)
   //    0
   //};

    // 5. 创建真正的 GL 3.3 上下文（最稳定）
    int attribs[] =
    {
        0x2091, 3,                      // major
        0x2092, 3,                      // minor
        0x9126, 0x00000001,             // core profile
        0
    };

    HGLRC realContext = nullptr;

    if (wglCreateContextAttribsARB)
    {
        realContext = wglCreateContextAttribsARB(hDC, 0, attribs);
    }

    hal::dout << "RealContext ptr: " << (void*)realContext << std::endl;
    hal::dout << "MakeCurrent Temp: " << (int)wglGetCurrentContext() << std::endl;

    // 如果失败，退回旧方式
    if (!realContext)
        realContext = tempContext;

    // 绑定真正的上下文
    wglMakeCurrent(nullptr, nullptr);
    wglMakeCurrent(hDC, realContext);
    hal::dout << "MakeCurrent Real: " << (int)wglGetCurrentContext() << std::endl;
    // 删除临时 context（如果不同）
    if (realContext != tempContext)
        wglDeleteContext(tempContext);

    this->glContext = realContext;





    hal::dout << "Ready to load GLAD...\n";


    // 6. GLAD 必须在真正 context 后加载
    bool ok = gladLoadGLLoader((GLADloadproc)[](const char* name) {
        void* p = (void*)wglGetProcAddress(name);
        if (p) return p;
        return (void*)GetProcAddress(GetModuleHandleA("opengl32.dll"), name);
    });

    hal::dout << "GLAD Load = " << ok << '\n';

    if (!ok) {
        MessageBoxA(nullptr, "GLAD 加载失败", "Error", MB_OK);
    }
    //if (!gladLoadGLLoader((GLADloadproc)wglGetProcAddress))
    //    MessageBoxA(nullptr, "GLAD 加载失败", "Error", MB_OK);

    // 7. 设置视口
    glViewport(0, 0, width, height);

    // 8. 基础渲染状态
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   

    // =====================================================
    // 8. 初始化 ImGui（OpenGL 版本）
    // =====================================================
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // 字体（可选）
    io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/msyh.ttc", 18.0f, nullptr,
        io.Fonts->GetGlyphRangesChineseFull());

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;    // ✅ 启用 Docking
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // 可选: 拖出成独立系统窗口

    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // ⭐ 开启多窗口

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hWnd);
    //ImGui_ImplOpenGL3_Init("#version 460");
    //ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplOpenGL3_Init("#version 330");

    // 启用 Win32 多窗口平台
    ImGui_ImplWin32_EnableDpiAwareness();

    // ⭐⭐ 很关键：让 ImGui 正确管理 Win32 的外置窗口，否则 GL 显示会错乱
    ImGui::GetMainViewport()->PlatformWindowCreated = true;

    // ⭐⭐ 加这一句可以避免拖出窗口黑屏
    ImGui_ImplWin32_EnableDpiAwareness();


    CreateGLBuffers();
    SetupVertexAttributes();
    //CreateGLInputLayout();
    CreateShaderProgram();
    SetupBlendState_OpenGL();
}


void Graphics_GL::ClearBuffer(float red, float green, float blue) noexcept
{
    glClearColor(red, green, blue, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void Graphics_GL::EndFrame()
{
    OutputDebugStringA("Graphics_GL::EndFrame called!\n");

    // ---------------------------
    // 1. 按 Z 排序（完全一样）
    // ---------------------------
    std::sort(object_2D_list_Darw.begin(), object_2D_list_Darw.end(),
        [](const Object_2D* a, const Object_2D* b)
        {
            return a->Z_Test < b->Z_Test;
        });

    // ---------------------------
    // 2. 循环绘制每个 2D 物体
    // ---------------------------
    for (auto* obj : object_2D_list_Darw)
    {
        // ---------------------------
        // 2-1. 绑定贴图（DX: PSSetShaderResources）
        // ---------------------------
        GLuint texID;
        if (obj->texNo < 0)
        {
            texID = 0;
        }
        else
        {
            texID = (GLuint)obj->texNo;  // 返回 OpenGL 的纹理 ID
        }


        if (texID != 0)
        {
            glBindTexture(GL_TEXTURE_2D, texID);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        // ---------------------------
        // 2-2. 更新顶点数据（DX: Map / Unmap）
        // ---------------------------
        Vertex_GL vertices[4];

        for (int i = 0; i < 4; i++)
        {
            vertices[i].position[0] = obj->Vertexs[i].x;
            vertices[i].position[1] = obj->Vertexs[i].y;
            vertices[i].position[2] = obj->Z_Test;
            vertices[i].position[3] = 1.0f;

            vertices[i].color[0] = obj->Color.R;
            vertices[i].color[1] = obj->Color.G;
            vertices[i].color[2] = obj->Color.B;
            vertices[i].color[3] = obj->Color.A;

            vertices[i].normal[0] = 0.0f;
            vertices[i].normal[1] = 0.0f;
            vertices[i].normal[2] = -1.0f;

            vertices[i].hasTexture = (obj->texNo >= 0) ? 1.0f : 0.0f;
        }

        // UV 翻转逻辑完全照搬 DX
        float u0 = obj->flipX ? obj->UV[1].x : obj->UV[0].x;
        float u1 = obj->flipX ? obj->UV[0].x : obj->UV[1].x;
        float v0 = obj->flipY ? obj->UV[2].y : obj->UV[0].y;
        float v1 = obj->flipY ? obj->UV[0].y : obj->UV[2].y;

        vertices[0].texcoord[0] = u0; vertices[0].texcoord[1] = v0;
        vertices[1].texcoord[0] = u1; vertices[1].texcoord[1] = v0;
        vertices[2].texcoord[0] = u0; vertices[2].texcoord[1] = v1;
        vertices[3].texcoord[0] = u1; vertices[3].texcoord[1] = v1;

        // ---------------------------
        // 2-3. 上传到 GPU（DX: Map；GL: glBufferSubData）
        // ---------------------------
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        // ---------------------------
        // 2-4. 绘制（DX: DrawIndexed）
        // ---------------------------
        Darw_GL();  // 你的 GL draw 函数，不变
    }

    // ---------------------------
    // 3. ImGui 渲染（GL 版本）
    // ---------------------------

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    // ---------------------------
    // 4. 清空列表
    // ---------------------------
    object_2D_list_Darw.clear();

    // ---------------------------
    // 5. SwapBuffers（DX: Present）
    // ---------------------------
    SwapBuffers(hDC);
}



void Graphics_GL::Darw_GL()
{
    glUseProgram(shaderProgram);      // 绑定 shader
    glBindVertexArray(VAO);           // 绑定顶点格式（等同 InputLayout）
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}


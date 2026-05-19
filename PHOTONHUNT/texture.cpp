#include "main.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <glad/glad.h>

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")
}

std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> g_Textures;


std::string ConvertWCharToUtf8(const wchar_t* wideStr) {
    if (!wideStr) return std::string();
    int len = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, nullptr, 0, nullptr, nullptr);
    std::string utf8str(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, &utf8str[0], len, nullptr, nullptr);
    // 去掉末尾的 '\0'
    if (!utf8str.empty() && utf8str.back() == '\0') utf8str.pop_back();
    return utf8str;
}

unsigned int LoadTexture(std::string FileName)
{

    //std::string Path = ConvertWCharToUtf8(FileName);
    std::string Path = FileName;

    for (size_t i = 0; i + 1 < Path.size(); ++i) {
        if (Path[i] == ':' && Path[i + 1] == '/') {
            Path.erase(i, 1);
        }
    }


    // 用于存储整个媒体文件的封装格式信息（如视频/音频流、元信息等）
    AVFormatContext* fmt_ctx = nullptr;

    // 指向解码器上下文，包含解码相关的配置，如分辨率、像素格式等
    AVCodecContext* codec_ctx = nullptr;

    // 指向具体的解码器（如 h264 解码器），用于初始化解码上下文
    const AVCodec* codec = nullptr;

    // 用于存储解码后的视频帧（一般为 YUV 格式）
    AVFrame* frame = nullptr;

    // 用于存储转换后的帧（如 RGBA 格式，便于显示或处理）
    AVFrame* frameRGBA = nullptr;

    // 图像转换上下文，用于从 YUV 转换为 RGBA 等目标格式
    SwsContext* sws_ctx = nullptr;

    // 用于暂存从媒体文件中读取的压缩数据包（如 h264 压缩视频帧）
    AVPacket* pkt = nullptr;

    // 打开文件（FFmpeg 会根据文件扩展名自动判断格式）

    if (avformat_open_input(&fmt_ctx, Path.c_str(), nullptr, nullptr) != 0) {
        std::string txt = "❌ Unable to open file: " + Path;
        MessageBoxW(nullptr, std::wstring(txt.begin(), txt.end()).c_str(), L"Runtime Error", MB_OK | MB_ICONEXCLAMATION);
    }

    // 读取媒体信息（必须执行）
    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
        std::string txt = "❌ Failed to retrieve media info.";
        MessageBoxW(nullptr, std::wstring(txt.begin(), txt.end()).c_str(), L"Runtime Error", MB_OK | MB_ICONEXCLAMATION);
        avformat_close_input(&fmt_ctx);
    }

    int width = 0;
    int height = 0;


    int stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    AVStream* stream = fmt_ctx->streams[stream_index];
    int64_t frameCount = stream->nb_frames;

    bool hasVideo = true;
    bool hasAudio = false;
    bool hasSubtitle = false;

    for (unsigned int i = 0; i < fmt_ctx->nb_streams; ++i) {
        AVMediaType type = fmt_ctx->streams[i]->codecpar->codec_type;
        if (type == AVMEDIA_TYPE_AUDIO) hasAudio = true;
        if (type == AVMEDIA_TYPE_SUBTITLE) hasSubtitle = true;
    }

    bool isSingleFrame = (frameCount <= 1); // 最稳妥的图像判断标准

    if (hasVideo && !hasAudio && !hasSubtitle && isSingleFrame) {

        // 寻找第一个视频流
        int stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
        if (stream_index < 0) {
            MessageBox(NULL, L"❌ Failed to find video stream", L"FFmpeg エラー", MB_OK | MB_ICONERROR);
            avformat_close_input(&fmt_ctx);
            return -1;
        }

        // 获取解码器参数
        AVCodecParameters* codecpar = fmt_ctx->streams[stream_index]->codecpar;

        codec = avcodec_find_decoder(codecpar->codec_id);
        codec_ctx = avcodec_alloc_context3(codec);
        avcodec_parameters_to_context(codec_ctx, codecpar);
        avcodec_open2(codec_ctx, codec, nullptr);

        frame = av_frame_alloc();        // 原始帧（YUV、MJPEG 等）
        frameRGBA = av_frame_alloc();    // 转换后帧（RGBA）
        pkt = av_packet_alloc();        // 压缩数据包

        int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, codec_ctx->width, codec_ctx->height, 1);
        uint8_t* buffer = (uint8_t*)av_malloc(numBytes);
        if (!buffer) {
            MessageBox(NULL, L"❌ メモリ割り当てに失敗しました！", L"メモリエラー", MB_OK | MB_ICONERROR);
            return -1;
        }

        av_image_fill_arrays(frameRGBA->data, frameRGBA->linesize, buffer, AV_PIX_FMT_RGBA,
            codec_ctx->width, codec_ctx->height, 1);


        SwsContext* sws_ctx = sws_getContext(codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
            codec_ctx->width, codec_ctx->height, AV_PIX_FMT_RGBA,
            SWS_BILINEAR, nullptr, nullptr, nullptr);

        bool success = false;

        while (av_read_frame(fmt_ctx, pkt) >= 0) {
            if (pkt->stream_index == stream_index) {
                if (avcodec_send_packet(codec_ctx, pkt) == 0 &&
                    avcodec_receive_frame(codec_ctx, frame) == 0) {

                    sws_scale(sws_ctx, frame->data, frame->linesize, 0,
                        codec_ctx->height, frameRGBA->data, frameRGBA->linesize);
                    width = codec_ctx->width;
                    height = codec_ctx->height;

                    if (frameRGBA->linesize[0] != codec_ctx->width * 4) {
                        MessageBox(NULL, L"⚠️ メモリが連続していません（コピーが必要）", L"警告", MB_OK | MB_ICONWARNING);
                    }

                    success = true;
                    break;
                }
            }
            av_packet_unref(pkt);
        }

        if (!success) {
            MessageBox(NULL, L"❌ 読み込みまたはデコードに失敗しました！", L"失敗", MB_OK | MB_ICONERROR);
        }

        // 清理（frameRGBA 和 buffer 不释放）
        av_packet_free(&pkt);
        av_frame_free(&frame);
        sws_freeContext(sws_ctx);
        avcodec_free_context(&codec_ctx);

    }



    avformat_close_input(&fmt_ctx);

    // 创建 GPU 贴图
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA texData = {};
    texData.pSysMem = frameRGBA->data[0];
    texData.SysMemPitch = frameRGBA->linesize[0];

    ID3D11Texture2D* texture = nullptr;


    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> g_Texture;


    HRESULT hr1 = Win->Gfx().pDevice->CreateTexture2D(&texDesc, &texData, &texture);
    if (FAILED(hr1)) {
        MessageBoxW(nullptr, L"❌ Texture2D 作成に失敗", L"DirectX エラー", MB_OK | MB_ICONERROR);
        return -1;
    }

    HRESULT hr2 = Win->Gfx().pDevice->CreateShaderResourceView(texture, nullptr, &g_Texture);
    if (FAILED(hr2) || !g_Texture) {
        MessageBoxW(nullptr, L"❌ ShaderResourceView 作成に失敗", L"DirectX エラー", MB_OK | MB_ICONERROR);
        return -1;
    }

    g_Textures.push_back(g_Texture);

    return g_Textures.size();
}


Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> LoadTexture_(std::string FileName)
{

    //std::string Path = ConvertWCharToUtf8(FileName);
    std::string Path = FileName;

    for (size_t i = 0; i + 1 < Path.size(); ++i) {
        if (Path[i] == ':' && Path[i + 1] == '/') {
            Path.erase(i, 1);
        }
    }


    // 用于存储整个媒体文件的封装格式信息（如视频/音频流、元信息等）
    AVFormatContext* fmt_ctx = nullptr;

    // 指向解码器上下文，包含解码相关的配置，如分辨率、像素格式等
    AVCodecContext* codec_ctx = nullptr;

    // 指向具体的解码器（如 h264 解码器），用于初始化解码上下文
    const AVCodec* codec = nullptr;

    // 用于存储解码后的视频帧（一般为 YUV 格式）
    AVFrame* frame = nullptr;

    // 用于存储转换后的帧（如 RGBA 格式，便于显示或处理）
    AVFrame* frameRGBA = nullptr;

    // 图像转换上下文，用于从 YUV 转换为 RGBA 等目标格式
    SwsContext* sws_ctx = nullptr;

    // 用于暂存从媒体文件中读取的压缩数据包（如 h264 压缩视频帧）
    AVPacket* pkt = nullptr;

    // 打开文件（FFmpeg 会根据文件扩展名自动判断格式）

    if (avformat_open_input(&fmt_ctx, Path.c_str(), nullptr, nullptr) != 0) {
        std::string txt = "❌ Unable to open file: " + Path;
        MessageBoxW(nullptr, std::wstring(txt.begin(), txt.end()).c_str(), L"Runtime Error", MB_OK | MB_ICONEXCLAMATION);
    }

    // 读取媒体信息（必须执行）
    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
        std::string txt = "❌ Failed to retrieve media info.";
        MessageBoxW(nullptr, std::wstring(txt.begin(), txt.end()).c_str(), L"Runtime Error", MB_OK | MB_ICONEXCLAMATION);
        avformat_close_input(&fmt_ctx);
    }

    int width = 0;
    int height = 0;


    int stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    AVStream* stream = fmt_ctx->streams[stream_index];
    int64_t frameCount = stream->nb_frames;

    bool hasVideo = true;
    bool hasAudio = false;
    bool hasSubtitle = false;

    for (unsigned int i = 0; i < fmt_ctx->nb_streams; ++i) {
        AVMediaType type = fmt_ctx->streams[i]->codecpar->codec_type;
        if (type == AVMEDIA_TYPE_AUDIO) hasAudio = true;
        if (type == AVMEDIA_TYPE_SUBTITLE) hasSubtitle = true;
    }

    bool isSingleFrame = (frameCount <= 1); // 最稳妥的图像判断标准

    if (hasVideo && !hasAudio && !hasSubtitle && isSingleFrame) {

        // 寻找第一个视频流
        int stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
        if (stream_index < 0) {
            MessageBox(NULL, L"❌ Failed to find video stream", L"FFmpeg エラー", MB_OK | MB_ICONERROR);
            avformat_close_input(&fmt_ctx);
            return NULL;
        }

        // 获取解码器参数
        AVCodecParameters* codecpar = fmt_ctx->streams[stream_index]->codecpar;

        codec = avcodec_find_decoder(codecpar->codec_id);
        codec_ctx = avcodec_alloc_context3(codec);
        avcodec_parameters_to_context(codec_ctx, codecpar);
        avcodec_open2(codec_ctx, codec, nullptr);

        frame = av_frame_alloc();        // 原始帧（YUV、MJPEG 等）
        frameRGBA = av_frame_alloc();    // 转换后帧（RGBA）
        pkt = av_packet_alloc();        // 压缩数据包

        int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, codec_ctx->width, codec_ctx->height, 1);
        uint8_t* buffer = (uint8_t*)av_malloc(numBytes);
        if (!buffer) {
            MessageBox(NULL, L"❌ メモリ割り当てに失敗しました！", L"メモリエラー", MB_OK | MB_ICONERROR);
            return NULL;
        }

        av_image_fill_arrays(frameRGBA->data, frameRGBA->linesize, buffer, AV_PIX_FMT_RGBA,
            codec_ctx->width, codec_ctx->height, 1);


        SwsContext* sws_ctx = sws_getContext(codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
            codec_ctx->width, codec_ctx->height, AV_PIX_FMT_RGBA,
            SWS_BILINEAR, nullptr, nullptr, nullptr);

        bool success = false;

        while (av_read_frame(fmt_ctx, pkt) >= 0) {
            if (pkt->stream_index == stream_index) {
                if (avcodec_send_packet(codec_ctx, pkt) == 0 &&
                    avcodec_receive_frame(codec_ctx, frame) == 0) {

                    sws_scale(sws_ctx, frame->data, frame->linesize, 0,
                        codec_ctx->height, frameRGBA->data, frameRGBA->linesize);
                    width = codec_ctx->width;
                    height = codec_ctx->height;

                    if (frameRGBA->linesize[0] != codec_ctx->width * 4) {
                        MessageBox(NULL, L"⚠️ メモリが連続していません（コピーが必要）", L"警告", MB_OK | MB_ICONWARNING);
                    }

                    success = true;
                    break;
                }
            }
            av_packet_unref(pkt);
        }

        if (!success) {
            MessageBox(NULL, L"❌ 読み込みまたはデコードに失敗しました！", L"失敗", MB_OK | MB_ICONERROR);
        }

        // 清理（frameRGBA 和 buffer 不释放）
        av_packet_free(&pkt);
        av_frame_free(&frame);
        sws_freeContext(sws_ctx);
        avcodec_free_context(&codec_ctx);

    }



    avformat_close_input(&fmt_ctx);

    // 创建 GPU 贴图
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA texData = {};
    texData.pSysMem = frameRGBA->data[0];
    texData.SysMemPitch = frameRGBA->linesize[0];

    ID3D11Texture2D* texture = nullptr;


    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> g_Texture;


    HRESULT hr1 = Win->Gfx().pDevice->CreateTexture2D(&texDesc, &texData, &texture);
    if (FAILED(hr1)) {
        MessageBoxW(nullptr, L"❌ Texture2D 作成に失敗", L"DirectX エラー", MB_OK | MB_ICONERROR);
        return NULL;
    }

    HRESULT hr2 = Win->Gfx().pDevice->CreateShaderResourceView(texture, nullptr, &g_Texture);
    if (FAILED(hr2) || !g_Texture) {
        MessageBoxW(nullptr, L"❌ ShaderResourceView 作成に失敗", L"DirectX エラー", MB_OK | MB_ICONERROR);
        return NULL;
    }

    return g_Texture;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> LoadTexture_(
    const float r,
    const float g, 
    const float b, 
    const float a)
{
    using Microsoft::WRL::ComPtr;

    // 1×1 RGBA8 颜色（0–255）
    unsigned char color[4] =
    {
        static_cast<unsigned char>(r),
        static_cast<unsigned char>(g),
        static_cast<unsigned char>(b),
        static_cast<unsigned char>(a)
    };

    // Texture 描述
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = 1;
    texDesc.Height = 1;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_IMMUTABLE;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = color;
    initData.SysMemPitch = 4; // 1 pixel * 4 bytes

    ComPtr<ID3D11Texture2D> texture;
    HRESULT hr = Win->Gfx().pDevice->CreateTexture2D(&texDesc, &initData, &texture);
    if (FAILED(hr))
        return nullptr;

    ComPtr<ID3D11ShaderResourceView> srv;
    hr = Win->Gfx().pDevice->CreateShaderResourceView(texture.Get(), nullptr, &srv);
    if (FAILED(hr))
        return nullptr;

    return srv;
}

unsigned int LoadTexture(std::string FileName, Object_2D::Vec2& size)
{

    //std::string Path = ConvertWCharToUtf8(FileName);
    std::string Path = FileName;

    for (size_t i = 0; i + 1 < Path.size(); ++i) {
        if (Path[i] == ':' && Path[i + 1] == '/') {
            Path.erase(i, 1);
        }
    }


    // 用于存储整个媒体文件的封装格式信息（如视频/音频流、元信息等）
    AVFormatContext* fmt_ctx = nullptr;

    // 指向解码器上下文，包含解码相关的配置，如分辨率、像素格式等
    AVCodecContext* codec_ctx = nullptr;

    // 指向具体的解码器（如 h264 解码器），用于初始化解码上下文
    const AVCodec* codec = nullptr;

    // 用于存储解码后的视频帧（一般为 YUV 格式）
    AVFrame* frame = nullptr;

    // 用于存储转换后的帧（如 RGBA 格式，便于显示或处理）
    AVFrame* frameRGBA = nullptr;

    // 图像转换上下文，用于从 YUV 转换为 RGBA 等目标格式
    SwsContext* sws_ctx = nullptr;

    // 用于暂存从媒体文件中读取的压缩数据包（如 h264 压缩视频帧）
    AVPacket* pkt = nullptr;

    // 打开文件（FFmpeg 会根据文件扩展名自动判断格式）

    if (avformat_open_input(&fmt_ctx, Path.c_str(), nullptr, nullptr) != 0) {
        std::string txt = "❌ Unable to open file: " + Path;
        MessageBoxW(nullptr, std::wstring(txt.begin(), txt.end()).c_str(), L"Runtime Error", MB_OK | MB_ICONEXCLAMATION);
    }

    // 读取媒体信息（必须执行）
    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
        std::string txt = "❌ Failed to retrieve media info.";
        MessageBoxW(nullptr, std::wstring(txt.begin(), txt.end()).c_str(), L"Runtime Error", MB_OK | MB_ICONEXCLAMATION);
        avformat_close_input(&fmt_ctx);
    }

    int width = 0;
    int height = 0;


    int stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    AVStream* stream = fmt_ctx->streams[stream_index];
    int64_t frameCount = stream->nb_frames;

    bool hasVideo = true;
    bool hasAudio = false;
    bool hasSubtitle = false;

    for (unsigned int i = 0; i < fmt_ctx->nb_streams; ++i) {
        AVMediaType type = fmt_ctx->streams[i]->codecpar->codec_type;
        if (type == AVMEDIA_TYPE_AUDIO) hasAudio = true;
        if (type == AVMEDIA_TYPE_SUBTITLE) hasSubtitle = true;
    }

    bool isSingleFrame = (frameCount <= 1); // 最稳妥的图像判断标准

    if (hasVideo && !hasAudio && !hasSubtitle && isSingleFrame) {

        // 寻找第一个视频流
        int stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
        if (stream_index < 0) {
            MessageBox(NULL, L"❌ Failed to find video stream", L"FFmpeg エラー", MB_OK | MB_ICONERROR);
            avformat_close_input(&fmt_ctx);
            return -1;
        }

        // 获取解码器参数
        AVCodecParameters* codecpar = fmt_ctx->streams[stream_index]->codecpar;

        codec = avcodec_find_decoder(codecpar->codec_id);
        codec_ctx = avcodec_alloc_context3(codec);
        avcodec_parameters_to_context(codec_ctx, codecpar);
        avcodec_open2(codec_ctx, codec, nullptr);

        frame = av_frame_alloc();        // 原始帧（YUV、MJPEG 等）
        frameRGBA = av_frame_alloc();    // 转换后帧（RGBA）
        pkt = av_packet_alloc();        // 压缩数据包

        int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, codec_ctx->width, codec_ctx->height, 1);
        uint8_t* buffer = (uint8_t*)av_malloc(numBytes);
        if (!buffer) {
            MessageBox(NULL, L"❌ メモリ割り当てに失敗しました！", L"メモリエラー", MB_OK | MB_ICONERROR);
            return -1;
        }

        av_image_fill_arrays(frameRGBA->data, frameRGBA->linesize, buffer, AV_PIX_FMT_RGBA,
            codec_ctx->width, codec_ctx->height, 1);


        SwsContext* sws_ctx = sws_getContext(codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
            codec_ctx->width, codec_ctx->height, AV_PIX_FMT_RGBA,
            SWS_BILINEAR, nullptr, nullptr, nullptr);

        bool success = false;

        while (av_read_frame(fmt_ctx, pkt) >= 0) {
            if (pkt->stream_index == stream_index) {
                if (avcodec_send_packet(codec_ctx, pkt) == 0 &&
                    avcodec_receive_frame(codec_ctx, frame) == 0) {

                    sws_scale(sws_ctx, frame->data, frame->linesize, 0,
                        codec_ctx->height, frameRGBA->data, frameRGBA->linesize);
                    width = codec_ctx->width;
                    height = codec_ctx->height;

                    if (frameRGBA->linesize[0] != codec_ctx->width * 4) {
                        MessageBox(NULL, L"⚠️ メモリが連続していません（コピーが必要）", L"警告", MB_OK | MB_ICONWARNING);
                    }

                    success = true;
                    break;
                }
            }
            av_packet_unref(pkt);
        }

        if (!success) {
            MessageBox(NULL, L"❌ 読み込みまたはデコードに失敗しました！", L"失敗", MB_OK | MB_ICONERROR);
        }

        // 清理（frameRGBA 和 buffer 不释放）
        av_packet_free(&pkt);
        av_frame_free(&frame);
        sws_freeContext(sws_ctx);
        avcodec_free_context(&codec_ctx);

    }



    avformat_close_input(&fmt_ctx);
    size.x = width;
    size.y = height;
    // 创建 GPU 贴图
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA texData = {};
    texData.pSysMem = frameRGBA->data[0];
    texData.SysMemPitch = frameRGBA->linesize[0];

    ID3D11Texture2D* texture = nullptr;


    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> g_Texture;


    HRESULT hr1 = Win->Gfx().pDevice->CreateTexture2D(&texDesc, &texData, &texture);
    if (FAILED(hr1)) {
        MessageBoxW(nullptr, L"❌ Texture2D 作成に失敗", L"DirectX エラー", MB_OK | MB_ICONERROR);
        return -1;
    }

    HRESULT hr2 = Win->Gfx().pDevice->CreateShaderResourceView(texture, nullptr, &g_Texture);
    if (FAILED(hr2) || !g_Texture) {
        MessageBoxW(nullptr, L"❌ ShaderResourceView 作成に失敗", L"DirectX エラー", MB_OK | MB_ICONERROR);
        return -1;
    }

    g_Textures.push_back(g_Texture);

    return g_Textures.size();
}



unsigned int LoadTexture_GL(std::string FileName, Object_2D::Vec2& size)
{
    // -----------------------------
    // 1. 用 FFmpeg 解码 RGBA（沿用你现有代码）
    // -----------------------------
    std::string Path = FileName;

    for (size_t i = 0; i + 1 < Path.size(); ++i) {
        if (Path[i] == ':' && Path[i + 1] == '/') {
            Path.erase(i, 1);
        }
    }

    AVFormatContext* fmt_ctx = nullptr;
    if (avformat_open_input(&fmt_ctx, Path.c_str(), nullptr, nullptr) != 0) {
        return -1;
    }

    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    int stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (stream_index < 0) {
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    AVCodecParameters* codecpar = fmt_ctx->streams[stream_index]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
    AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codec_ctx, codecpar);
    avcodec_open2(codec_ctx, codec, nullptr);

    AVFrame* frame = av_frame_alloc();
    AVFrame* frameRGBA = av_frame_alloc();
    AVPacket* pkt = av_packet_alloc();

    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, codec_ctx->width, codec_ctx->height, 1);
    uint8_t* buffer = (uint8_t*)av_malloc(numBytes);
    av_image_fill_arrays(frameRGBA->data, frameRGBA->linesize, buffer,
        AV_PIX_FMT_RGBA, codec_ctx->width, codec_ctx->height, 1);

    SwsContext* sws_ctx = sws_getContext(
        codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
        codec_ctx->width, codec_ctx->height, AV_PIX_FMT_RGBA,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );

    bool success = false;
    while (av_read_frame(fmt_ctx, pkt) >= 0)
    {
        if (pkt->stream_index == stream_index) {
            if (avcodec_send_packet(codec_ctx, pkt) == 0 &&
                avcodec_receive_frame(codec_ctx, frame) == 0) {

                sws_scale(sws_ctx,
                    frame->data, frame->linesize,
                    0, codec_ctx->height,
                    frameRGBA->data, frameRGBA->linesize);

                size.x = codec_ctx->width;
                size.y = codec_ctx->height;

                success = true;
                break;
            }
        }
        av_packet_unref(pkt);
    }

    av_packet_free(&pkt);
    av_frame_free(&frame);
    sws_freeContext(sws_ctx);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&fmt_ctx);

    if (!success) return -1;

    // -----------------------------
    // 2. OpenGL: 创建 GPU 纹理
    // -----------------------------
    GLuint texID = 0;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        size.x,
        size.y,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        frameRGBA->data[0]
    );

    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    return (int)texID;
    //return texID;
}


void UnloadTexture(unsigned int Texture)
{
    if (Texture != -1) {
        //glDeleteTextures(1, &Texture);
    }
}





ID3D11ShaderResourceView* SetTexture(unsigned int Texture)
{

    if (Texture <= g_Textures.size() && Texture > 0)
    {
        return g_Textures[Texture - 1].Get();
    }
    return NULL;
}


std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>& GetTexture()
{
    return g_Textures;
}



Float2 MakeFloat2(float x, float y)
{
    return { x, y, 0 };
}

Float3 MakeFloat3(float x, float y, float z)
{
    return { x, y, z, 0 };
}

Float4 MakeFloat4(float x, float y, float z, float w)
{
    return { x, y, z, w };
}

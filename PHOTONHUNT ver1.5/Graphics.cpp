#include "Graphics.h"
#include "shader.h"
#include "main.h"
#include "texture.h"
#include "Utility.h"
#include "collision.h"
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

Light light;
float bFactor[4] = { 0.0f,0.0f,0.0f,0.0f };
ID3D11BlendState* bState[BLENDSTATE::BLENDSTATE_MAX];
ID3D11DepthStencilState* g_DepthStateEnable;
ID3D11DepthStencilState* g_DepthStateDisable;

Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader_3D_Pixelation;//像素化像素

Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer_uv; // 顶点缓冲区句柄/索引
Microsoft::WRL::ComPtr<ID3D11Buffer> pIndexBuffer_uv; // 索引缓冲区句柄/索引

// 既存の sceneDSV とは別に2D用として追加
Microsoft::WRL::ComPtr<ID3D11Texture2D>        p2DDepthStencilTex;
Microsoft::WRL::ComPtr<ID3D11DepthStencilView> p2DDSV;

// ステンシル操作用のステートも追加
Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pStencilWriteState;  // 床を書き込む
Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pStencilMaskState;   // 影をマスクする
Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pStencilOffState;    // ステンシル無効

std::string WCharToString(const wchar_t* wstr) {
	int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
	std::string result(len, '\0');
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &result[0], len, nullptr, nullptr);
	return result;
}


Graphics::Graphics( HWND hWnd ,int w, int h)
{
	width = w;
	height = h;
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = hWnd;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	// GPU 情報を取得するための DXGI ファクトリーを作成
	IDXGIFactory* pFactory = nullptr;
	if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory))) {
		//std::cerr << "DXGIファクトリーの作成に失敗しました。" << std::endl;
	}

	// 利用可能な GPU アダプターを列挙
	IDXGIAdapter* pAdapter = nullptr;
	int index = 0;

	IDXGIAdapter* firstAdapter = nullptr;    // 最初に見つかった GPU
	IDXGIAdapter* discreteAdapter = nullptr; // NVIDIA や AMD などの専用 GPU


	while (pFactory->EnumAdapters(index, &pAdapter) != DXGI_ERROR_NOT_FOUND) {
		// アダプターの情報を取得
		DXGI_ADAPTER_DESC desc;
		pAdapter->GetDesc(&desc);

		// GPU 名を UTF-8 に変換
		std::string gpuName = WCharToString(desc.Description);

		// 最初のアダプターを記録（後で使う）
		if (index == 0) {
			firstAdapter = pAdapter;
			firstAdapter->AddRef(); // COM 管理のため参照カウントを追加
		}

		// 専用 GPU を記録（NVIDIA: 0x10DE, AMD: 0x1002）
		if ((desc.VendorId == 0x10DE || desc.VendorId == 0x1002) && discreteAdapter == nullptr) {
			discreteAdapter = pAdapter;
			discreteAdapter->AddRef(); // COM 管理のため参照カウントを追加
		}

		// 現在のアダプターの参照を解放
		pAdapter->Release();
		index++;
	}

	// ファクトリーの解放
	pFactory->Release();

	// 使用する GPU を選択（専用 GPU があればそれを優先）
	if (discreteAdapter) {
		DXGI_ADAPTER_DESC desc;
		discreteAdapter->GetDesc(&desc);
		std::string selectedName = WCharToString(desc.Description);
	}
	else if (firstAdapter) {
		DXGI_ADAPTER_DESC desc;
		firstAdapter->GetDesc(&desc);
		std::string selectedName = WCharToString(desc.Description);
	}
	else {
	}

	// 実際に使用するアダプターを決定
	IDXGIAdapter* selectedAdapter = (discreteAdapter != nullptr) ? discreteAdapter : firstAdapter;


	D3D11CreateDeviceAndSwapChain(
		selectedAdapter,
		D3D_DRIVER_TYPE_UNKNOWN,
		nullptr,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&sd,
		&pSwap,
		&pDevice,
		nullptr,
		&pContext
	);

	Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer;
	pSwap->GetBuffer(0, __uuidof(ID3D11Texture2D), &pBackBuffer);
	pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &pTarget);
	pBackBuffer->Release();

	InitBlendStates();
	Shader_Initialize(pDevice, pContext);
	CreateRasterizerState(pDevice, pContext);
	CreateViewport(pDevice, pContext, width, height);


	IMGUI_CHECKVERSION();
	ImGui::CreateContext();                // MUST HAVE!!!
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();

	// 字体（可选）
	io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/msyh.ttc", 18.0f, nullptr,
		io.Fonts->GetGlyphRangesChineseFull());
	//ImGui::StyleColorsDark();

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;    // ✅ 启用 Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // 可选: 拖出成独立系统窗口


	io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;

	// 3. 配置样式
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(pDevice.Get(), pContext.Get());

	depth();
}

void Graphics::EndFrame()
{
	{
		if (!object_3D_list_Darw_Opaque.empty())
		{
			Object_2D Opaque(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			BoxVertexsMake(&Opaque);
			Win->Gfx().pContext->PSSetShaderResources(0, 1, sceneOpaqueSRV.GetAddressOf());

			D3D11_MAPPED_SUBRESOURCE mappedRes{};

			HRESULT hr = Win->Gfx().pContext->Map(g_pVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes);


			Vector* vertexData = reinterpret_cast<Vector*>(mappedRes.pData);

			float cx = SCREEN_WIDTH * 0.5f;
			float cy = SCREEN_HEIGHT * 0.5f;


			for (int i = 0; i < 4; i++)
			{
				vertexData[i].position = { Opaque.Vertexs[i].x,
										   Opaque.Vertexs[i].y,
										   Opaque.Z_Test,0.0f };

				vertexData[i].color = { Opaque.Color.R, Opaque.Color.G, Opaque.Color.B, Opaque.Color.A };
				vertexData[i].normal = { 0.0f, 0.0f, -1.0f };
				vertexData[i].hasTexture = (Opaque.texNo >= 0) ? 1.0f : 0.0f;
			}


			float u0 = Opaque.flipX ? Opaque.UV[1].x : Opaque.UV[0].x;
			float u1 = Opaque.flipX ? Opaque.UV[0].x : Opaque.UV[1].x;
			float v0 = Opaque.flipY ? Opaque.UV[2].y : Opaque.UV[0].y;
			float v1 = Opaque.flipY ? Opaque.UV[0].y : Opaque.UV[2].y;

			vertexData[0].texcoord = { u0, v0 };
			vertexData[1].texcoord = { u1, v0 };
			vertexData[2].texcoord = { u0, v1 };
			vertexData[3].texcoord = { u1, v1 };


			pContext->Unmap(g_pVertexBuffer.Get(), 0);




			//SetBlendState(BLENDSTATE::BLENDSTATE_PREMULTIPLIED);
			SetBlendState(BLENDSTATE::BLENDSTATE_ALFA);

			const UINT stride = sizeof(Vector);
			const UINT offset = 0u;
			pContext->IASetVertexBuffers(0u, 1u, g_pVertexBuffer.GetAddressOf(), &stride, &offset);
			pContext->IASetIndexBuffer(
				g_pIndexBuffer.Get(),
				DXGI_FORMAT_R16_UINT,
				0u
			);



			pContext->VSSetShader(g_pVertexShader.Get(), nullptr, 0u);
			pContext->PSSetShader(pPixelShader_3D_Pixelation.Get(), nullptr, 0u);
			pContext->IASetInputLayout(g_pInputLayout.Get());
			pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), nullptr);
			pContext->RSSetState(pRasterizerState.Get());
			ResetConstantBuffer(pContext.Get());
			pContext->VSSetConstantBuffers(0u, 1u, g_pConstantBuffer.GetAddressOf());
			pContext->RSSetViewports(1u, &(*viewport));

			pContext->DrawIndexed(6, 0u, 0u);

			object_3D_list_Darw_Opaque.clear();

			if (!object_3D_list_Darw_Transparent.empty())
			{
				Object_2D Transparent(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
				BoxVertexsMake(&Transparent);
				Win->Gfx().pContext->PSSetShaderResources(0, 1, sceneTransparentSRV.GetAddressOf());

				Draw(nullptr);
				object_3D_list_Darw_Transparent.clear();
			}
		}
		
	}


	std::sort(object_2D_list_Darw.begin(), object_2D_list_Darw.end(),
		[](const Object_2D* a, const Object_2D* b)
		{
			return a->Z_Test < b->Z_Test;
		});

	// ループ前にステンシルをクリア（0にリセット）
	pContext->ClearDepthStencilView(p2DDSV.Get(), D3D11_CLEAR_STENCIL, 1.0f, 0);

	D3D11_RECT floorScissorRect = { 0, 0, 1920, 1080 }; // デフォルト値

	for (auto* obj : object_2D_list_Darw)
	{
		// ステンシル状態を切り替え
		if (obj->stencilRole == StencilRole::FloorWrite){

			//float blendFactor[4] = {};
			//pContext->OMSetBlendState(pNoColorWriteBlend.Get(), blendFactor, 0xffffffff);
			//pContext->OMSetDepthStencilState(pStencilWriteState.Get(), 1);
			//// 矩形をそのままDarw()で描画（cachedVertexBufferは使わない）
			//Darw(p2DDSV.Get());

			//// ビジュアル描画
			//pContext->OMSetDepthStencilState(pStencilOffState.Get(), 0);
			//SetBlendState(BLENDSTATE::BLENDSTATE_ALFA);
			//// 通常描画に流れる

			//float minX = obj->Vertexs[0].x;
			//float maxX = obj->Vertexs[0].x;
			//float minY = obj->Vertexs[0].y;
			//float maxY = obj->Vertexs[0].y;
			//for (int i = 1; i < 4; i++)
			//{
			//	if (obj->Vertexs[i].x < minX) minX = obj->Vertexs[i].x;
			//	if (obj->Vertexs[i].x > maxX) maxX = obj->Vertexs[i].x;
			//	if (obj->Vertexs[i].y < minY) minY = obj->Vertexs[i].y;
			//	if (obj->Vertexs[i].y > maxY) maxY = obj->Vertexs[i].y;
			//}

			//// ワールド座標 → ピクセル座標に変換
			//// x: -960～960 → 0～1920
			//// y: 540～-540 → 0～1080（Y軸反転）
			//floorScissorRect = {
			//	(LONG)(minX + 960.0f),          // 左
			//	(LONG)(-maxY + 540.0f),         // 上（Y反転）
			//	(LONG)(maxX + 960.0f),          // 右
			//	(LONG)(-minY + 540.0f)          // 下（Y反転）
			//};

			if (!obj->vt.empty())
			{
				float minX = obj->vt[0].x;
				float maxX = obj->vt[0].x;
				float minY = obj->vt[0].y;
				float maxY = obj->vt[0].y;
				for (auto& v : obj->vt)
				{
					if (v.x < minX) minX = v.x;
					if (v.x > maxX) maxX = v.x;
					if (v.y < minY) minY = v.y;
					if (v.y > maxY) maxY = v.y;
				}

				// UV(0~1) → ワールド座標 → ピクセル座標
				float worldMinX = (minX - 0.5f) * obj->Size.x + obj->Transform.Position.x;
				float worldMaxX = (maxX - 0.5f) * obj->Size.x + obj->Transform.Position.x;
				float worldMinY = (minY - 0.5f) * obj->Size.y + obj->Transform.Position.y;
				float worldMaxY = (maxY - 0.5f) * obj->Size.y + obj->Transform.Position.y;

				floorScissorRect = {
					(LONG)(worldMinX * WinCamera.Scale - WinCamera.pos.x + 960.0f),
					(LONG)(540.0f - worldMaxY * WinCamera.Scale + WinCamera.pos.y),
					(LONG)(worldMaxX * WinCamera.Scale - WinCamera.pos.x + 960.0f),
					(LONG)(540.0f - worldMinY * WinCamera.Scale + WinCamera.pos.y)
				};
			}

			float blendFactor[4] = {};
			pContext->OMSetBlendState(pNoColorWriteBlend.Get(), blendFactor, 0xffffffff);
			pContext->OMSetDepthStencilState(pStencilWriteState.Get(), 1); // ref=1

			if (obj->bufferReady)
			{
				const UINT stride = sizeof(Vector);
				const UINT offset = 0u;
				pContext->IASetVertexBuffers(0u, 1u, obj->cachedVertexBuffer.GetAddressOf(), &stride, &offset);
				pContext->IASetIndexBuffer(obj->cachedIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);
				pContext->VSSetShader(g_pVertexShader.Get(), nullptr, 0u);
				pContext->PSSetShader(g_pPixelShader.Get(), nullptr, 0u);
				pContext->IASetInputLayout(g_pInputLayout.Get());
				pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), p2DDSV.Get());
				pContext->RSSetState(pRasterizerState.Get());
				ResetConstantBuffer(pContext.Get());
				pContext->VSSetConstantBuffers(0u, 1u, g_pConstantBuffer.GetAddressOf());
				pContext->RSSetViewports(1u, &(*viewport));
				pContext->DrawIndexed(obj->cachedIndexCount, 0u, 0u);
			}
			pContext->OMSetDepthStencilState(pStencilOffState.Get(), 0);
			SetBlendState(BLENDSTATE::BLENDSTATE_ALFA);
		}
		else if (obj->stencilRole == StencilRole::ShadowMask){
			// 影 → ステンシルが0の場所だけ描画（ref=0と一致）
			pContext->OMSetDepthStencilState(pStencilMaskState.Get(), 1);
			pContext->RSSetScissorRects(1, &floorScissorRect);
		}
		else{
			// 通常オブジェクト → ステンシル無効
			pContext->OMSetDepthStencilState(pStencilOffState.Get(), 0);
		}

		if (obj->spine != -1)
		{
			DrawSpine(obj->spine);
			continue;
		}
		ID3D11ShaderResourceView* g_Texture = SetTexture(obj->texNo);
		if (g_Texture != NULL)
		{
			Win->Gfx().pContext->PSSetShaderResources(0, 1, &g_Texture);
		}
		else
		{
			ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
			Win->Gfx().pContext->PSSetShaderResources(0, 1, nullSRV);
		}


		D3D11_MAPPED_SUBRESOURCE mappedRes{};

		HRESULT hr = Win->Gfx().pContext->Map(g_pVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes);


		Vector* vertexData = reinterpret_cast<Vector*>(mappedRes.pData);

		float cx = SCREEN_WIDTH * 0.5f;
		float cy = SCREEN_HEIGHT * 0.5f;


		for (int i = 0; i < 4; i++)
		{
			vertexData[i].position = { obj->Vertexs[i].x,
									   obj->Vertexs[i].y,
									   obj->Z_Test,0.0f };

			vertexData[i].color = { obj->Color.R, obj->Color.G, obj->Color.B, obj->Color.A };
			vertexData[i].normal = { 0.0f, 0.0f, -1.0f };
			vertexData[i].hasTexture = (obj->texNo >= 0) ? 1.0f : 0.0f;
		}


		float u0 = obj->flipX ? obj->UV[1].x : obj->UV[0].x;
		float u1 = obj->flipX ? obj->UV[0].x : obj->UV[1].x;
		float v0 = obj->flipY ? obj->UV[2].y : obj->UV[0].y;
		float v1 = obj->flipY ? obj->UV[0].y : obj->UV[2].y;

		vertexData[0].texcoord = { u0, v0 };
		vertexData[1].texcoord = { u1, v0 };
		vertexData[2].texcoord = { u0, v1 };
		vertexData[3].texcoord = { u1, v1 };


		pContext->Unmap(g_pVertexBuffer.Get(), 0);
		std::vector<ID3D11Buffer*> rawBuffers;

		for (auto& cb : obj->g_pPSConstantBuffers)
		{
			rawBuffers.push_back(cb.Get());
		}

		if (!rawBuffers.empty())
		{
			pContext->PSSetConstantBuffers(
				0,
				static_cast<UINT>(rawBuffers.size()),
				rawBuffers.data()
			);
		}
		else
		{
			// 没有常量需求 → 直接清空 b0
			ID3D11Buffer* nullBuffer = nullptr;
			pContext->PSSetConstantBuffers(0, 1, &nullBuffer);
		}

		
		if (obj->g_pVertexShader == nullptr)
		{
			pContext->VSSetShader(g_pVertexShader.Get(), nullptr, 0u);
		}
		else
		{
			pContext->VSSetShader(obj->g_pVertexShader.Get(), nullptr, 0u);
		}

		if (obj->g_pPixelShader == nullptr)
		{
			pContext->PSSetShader(g_pPixelShader.Get(), nullptr, 0u);
		}
		else
		{
			pContext->PSSetShader(obj->g_pPixelShader.Get(), nullptr, 0u);
		}


		if (obj->stencilRole == StencilRole::ShadowMask)
		{
			Draw(p2DDSV.Get(), pRasterizerState_Scissor.Get());
		}
		else
		{
			Draw(p2DDSV.Get());
		}
	}

	// ループ後にステンシル無効に戻す
	pContext->OMSetDepthStencilState(pStencilOffState.Get(), 0);
	//pContext->RSSetState(pRasterizerState.Get()); // ← 元に戻す

	for (auto* obj : collision_2D_list_Draw)
	{
		if (!obj->showPolyCollision)continue;
		pIndexBuffer_uv = nullptr;
		pVertexBuffer_uv = nullptr;
		SetBlendState(BLENDSTATE::BLENDSTATE_ALFA);

		std::vector<Object_2D::Vec2> vt;
		std::vector<Vector> vx;


		if (!obj->Indices_vt.empty())
		{
			//ConvertUVtoModel(*obj, &vt);

			//for (auto& v : vt)
			//{
			//	Vector vv;

			//	vv.position = { (float)v.x,(float)v.y,0,1 };   // 位置
			//	vv.color = { 1,1,1,1 };      // 颜色
			//	vv.texcoord = { 0,0 };   // 纹理坐标
			//	vv.normal = { 0,0,0 };     // 法线
			//	vv.hasTexture = 0;            // 是否使用贴图 (1.0f = 有, 0.0f = 无)
			//	vx.push_back(vv);
			//}


			//// 设置顶点缓冲区描述
			//D3D11_BUFFER_DESC vertexBufferDesc = {};
			//vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			//vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			//vertexBufferDesc.ByteWidth = static_cast<UINT>(vx.size() * sizeof(Vector));
			//vertexBufferDesc.StructureByteStride = 0;

			//D3D11_SUBRESOURCE_DATA vertexDataDesc = {};
			//vertexDataDesc.pSysMem = vx.data();
			//if (FAILED(Win->Gfx().pDevice->CreateBuffer(&vertexBufferDesc, &vertexDataDesc, pVertexBuffer_uv.GetAddressOf())))
			//{
			//	MessageBox(NULL, L"Failed to create vertex buffer.", L"Error", MB_OK | MB_ICONERROR);
			//}


			//D3D11_BUFFER_DESC indexBufferDesc = {};
			//D3D11_SUBRESOURCE_DATA indexData = {};
			//indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			//indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			//indexBufferDesc.CPUAccessFlags = 0u;
			//indexBufferDesc.ByteWidth = obj->Indices_vt.size() * sizeof(uint16_t);
			//indexBufferDesc.StructureByteStride = sizeof(uint16_t);

			//indexData.pSysMem = obj->Indices_vt.data();

			//if (FAILED(Win->Gfx().pDevice->CreateBuffer(&indexBufferDesc, &indexData, pIndexBuffer_uv.GetAddressOf()))) {
			//	MessageBox(NULL, L"Failed to create index buffer.", L"Error", MB_OK | MB_ICONERROR);
			//}




			//const UINT stride = sizeof(Vector);
			//const UINT offset = 0u;
			//pContext->IASetVertexBuffers(0u, 1u, pVertexBuffer_uv.GetAddressOf(), &stride, &offset);
			//pContext->IASetIndexBuffer(
			//	pIndexBuffer_uv.Get(),
			//	DXGI_FORMAT_R16_UINT,
			//	0u
			//);

			// キャッシュ済みバッファがあればそれを使う
			if (obj->bufferReady)
			{
				const UINT stride = sizeof(Vector);
				const UINT offset = 0u;
				pContext->IASetVertexBuffers(0u, 1u, obj->cachedVertexBuffer.GetAddressOf(), &stride, &offset);
				pContext->IASetIndexBuffer(obj->cachedIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);

				pContext->VSSetShader(g_pVertexShader.Get(), nullptr, 0u);
				pContext->PSSetShader(g_pPixelShader_uv.Get(), nullptr, 0u);
				pContext->IASetInputLayout(g_pInputLayout.Get());
				pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), nullptr);
				pContext->RSSetState(pRasterizerState_uv.Get());
				pContext->VSSetConstantBuffers(0u, 1u, g_pConstantBuffer.GetAddressOf());
				pContext->RSSetViewports(1u, &(*viewport));


				pContext->DrawIndexed(obj->Indices_vt.size(), 0u, 0u);
			}
			else
			{
				// 毎フレーム頂点データだけ上書き
				ConvertUVtoModel(*obj, &vt);
				std::vector<Vector> vx;
				for (auto& v : vt) {
					Vector vv;
					vv.position = { v.x, v.y, 0, 1 };
					vv.color = { 1,1,1,1 };
					vv.texcoord = { 0,0 };
					vv.normal = { 0,0,0 };
					vv.hasTexture = 0;
					vx.push_back(vv);
				}

				D3D11_MAPPED_SUBRESOURCE mappedRes{};
				pContext->Map(obj->cachedVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes);
				memcpy(mappedRes.pData, vx.data(), vx.size() * sizeof(Vector));
				pContext->Unmap(obj->cachedVertexBuffer.Get(), 0);

				const UINT stride = sizeof(Vector);
				const UINT offset = 0u;
				pContext->IASetVertexBuffers(0u, 1u, obj->cachedVertexBuffer.GetAddressOf(), &stride, &offset);
				pContext->IASetIndexBuffer(obj->cachedIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);
				pContext->VSSetShader(g_pVertexShader.Get(), nullptr, 0u);
				pContext->PSSetShader(g_pPixelShader_uv.Get(), nullptr, 0u);
				pContext->IASetInputLayout(g_pInputLayout.Get());
				pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), nullptr);
				pContext->RSSetState(pRasterizerState_uv.Get());
				pContext->VSSetConstantBuffers(0u, 1u, g_pConstantBuffer.GetAddressOf());
				pContext->RSSetViewports(1u, &(*viewport));
				pContext->DrawIndexed(obj->cachedIndexCount, 0u, 0u);
			}
		}
		else {
			ID3D11ShaderResourceView* g_Texture = SetTexture(obj->texNo);
			if (g_Texture != NULL)
			{
				Win->Gfx().pContext->PSSetShaderResources(0, 1, &g_Texture);
			}
			else
			{
				ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
				Win->Gfx().pContext->PSSetShaderResources(0, 1, nullSRV);
			}


			D3D11_MAPPED_SUBRESOURCE mappedRes{};

			HRESULT hr = Win->Gfx().pContext->Map(g_pVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes);


			Vector* vertexData = reinterpret_cast<Vector*>(mappedRes.pData);

			float cx = SCREEN_WIDTH * 0.5f;
			float cy = SCREEN_HEIGHT * 0.5f;


			for (int i = 0; i < 4; i++)
			{
				vertexData[i].position = { obj->Vertexs[i].x,
										   obj->Vertexs[i].y,
										   obj->Z_Test,0.0f };

				vertexData[i].color = { obj->Color.R, obj->Color.G, obj->Color.B, obj->Color.A };
				vertexData[i].normal = { 0.0f, 0.0f, -1.0f };
				vertexData[i].hasTexture = (obj->texNo >= 0) ? 1.0f : 0.0f;
			}


			float u0 = obj->flipX ? obj->UV[1].x : obj->UV[0].x;
			float u1 = obj->flipX ? obj->UV[0].x : obj->UV[1].x;
			float v0 = obj->flipY ? obj->UV[2].y : obj->UV[0].y;
			float v1 = obj->flipY ? obj->UV[0].y : obj->UV[2].y;

			vertexData[0].texcoord = { u0, v0 };
			vertexData[1].texcoord = { u1, v0 };
			vertexData[2].texcoord = { u0, v1 };
			vertexData[3].texcoord = { u1, v1 };


			pContext->Unmap(g_pVertexBuffer.Get(), 0);


			const UINT stride = sizeof(Vector);
			const UINT offset = 0u;
			pContext->IASetVertexBuffers(0u, 1u, g_pVertexBuffer.GetAddressOf(), &stride, &offset);
			pContext->IASetIndexBuffer(
				g_pIndexBuffer.Get(),
				DXGI_FORMAT_R16_UINT,
				0u
			);



			pContext->VSSetShader(g_pVertexShader.Get(), nullptr, 0u);
			pContext->PSSetShader(g_pPixelShader_uv.Get(), nullptr, 0u);
			pContext->IASetInputLayout(g_pInputLayout.Get());
			pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), nullptr);
			pContext->RSSetState(pRasterizerState_uv.Get());
			pContext->VSSetConstantBuffers(0u, 1u, g_pConstantBuffer.GetAddressOf());
			pContext->RSSetViewports(1u, &(*viewport));

			pContext->DrawIndexed(6, 0u, 0u);
		}
	}
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// 支持多视口 (Viewports)
	ImGuiIO& io = ImGui::GetIO();
	// ☆☆☆ 如果你启用了 Viewport，一定要加下面两句
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
	object_2D_list_Darw.clear(); // 清空，防止重复绘制
	inti_Object_2D_spine();

	pSwap->Present(1u, 0u);
}

void Graphics::ClearBuffer(float red, float green, float blue) noexcept
{
	const float color[] = { red, green, blue, 1.0f };
	pContext->ClearRenderTargetView(pTarget.Get(), color);
}

// 初期化時に一度だけ呼ぶ
void Graphics::CreateDynamicCollisionBuffer(Object_2D* obj)
{
	// 頂点バッファ（DYNAMIC）
	D3D11_BUFFER_DESC vbd = {};
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.ByteWidth = 4 * sizeof(Vector); // 4頂点固定
	Win->Gfx().pDevice->CreateBuffer(&vbd, nullptr, obj->cachedVertexBuffer.GetAddressOf());

	// インデックスバッファは変わらないのでDEFAULTでOK
	D3D11_BUFFER_DESC ibd = {};
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = obj->Indices_vt.size() * sizeof(uint16_t);
	ibd.StructureByteStride = sizeof(uint16_t);
	D3D11_SUBRESOURCE_DATA id = {};
	id.pSysMem = obj->Indices_vt.data();
	Win->Gfx().pDevice->CreateBuffer(&ibd, &id, obj->cachedIndexBuffer.GetAddressOf());

	obj->cachedIndexCount = (int)obj->Indices_vt.size();
	//obj->bufferReady = true;
}

void Graphics::depth()
{
	D3D11_TEXTURE2D_DESC depthStencilDesc = {};
	depthStencilDesc.Width = width;  
	depthStencilDesc.Height = height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1; 
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &p2DDepthStencilTex); // ← メンバに保存

	// --- DSV作成 ---
	pDevice->CreateDepthStencilView(p2DDepthStencilTex.Get(), nullptr, &p2DDSV); // ← メンバに保存

	// --- 床をステンシルに書き込む用（ref=1を書く、描画はしない）---
	D3D11_DEPTH_STENCIL_DESC writeDesc = {};
	writeDesc.DepthEnable = false;               // 2Dなので深度不要
	writeDesc.StencilEnable = true;
	writeDesc.StencilReadMask = 0xFF;
	writeDesc.StencilWriteMask = 0xFF;
	writeDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS; // 常に書く
	writeDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;// ref値を書き込む
	writeDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	writeDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	writeDesc.BackFace = writeDesc.FrontFace;
	pDevice->CreateDepthStencilState(&writeDesc, &pStencilWriteState);

	// --- 影描画時マスク用（ステンシルが0の場所だけ描画）---
	D3D11_DEPTH_STENCIL_DESC maskDesc = {};
	maskDesc.DepthEnable = false;
	maskDesc.StencilEnable = true;
	maskDesc.StencilReadMask = 0xFF;
	maskDesc.StencilWriteMask = 0x00;                  // ステンシルは書き換えない
	maskDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;//refが１の以外
	maskDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	maskDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	maskDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	maskDesc.BackFace = maskDesc.FrontFace;
	pDevice->CreateDepthStencilState(&maskDesc, &pStencilMaskState);

	// --- ステンシル完全無効用 ---
	D3D11_DEPTH_STENCIL_DESC offDesc = {};
	offDesc.DepthEnable = false;
	offDesc.StencilEnable = false;
	pDevice->CreateDepthStencilState(&offDesc, &pStencilOffState);

	// RTVにDSVをバインド
	pContext->OMSetRenderTargets(1, pTarget.GetAddressOf(), p2DDSV.Get());

	D3D11_BLEND_DESC noColorDesc = {};
	noColorDesc.RenderTarget[0].BlendEnable = FALSE;
	noColorDesc.RenderTarget[0].RenderTargetWriteMask = 0;
	pDevice->CreateBlendState(&noColorDesc, &pNoColorWriteBlend);

	// シザーテスト有効のラスタライザステート
	D3D11_RASTERIZER_DESC scissorDesc = {};
	scissorDesc.FillMode = D3D11_FILL_SOLID;
	scissorDesc.CullMode = D3D11_CULL_NONE;
	scissorDesc.ScissorEnable = TRUE; // ← シザーテスト有効
	pDevice->CreateRasterizerState(&scissorDesc, &pRasterizerState_Scissor);

	/*Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthStencil;
	pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &pDepthStencil);

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepthStencilView;
	pDevice->CreateDepthStencilView(static_cast<ID3D11Resource*>(pDepthStencil.Get()), nullptr, &pDepthStencilView);

	D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc = {};
	depthStencilStateDesc.DepthEnable = true;                          
	depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; 
	depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;           

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDepthStencilState;
	pDevice->CreateDepthStencilState(&depthStencilStateDesc, &pDepthStencilState);

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pRenderTargetView;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer;
	pSwap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)pBackBuffer.GetAddressOf());

	pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &pRenderTargetView);

	pContext->OMSetRenderTargets(1, pRenderTargetView.GetAddressOf(), pDepthStencilView.Get());

	pContext->OMSetDepthStencilState(pDepthStencilState.Get(), 1);*/
}


void Graphics::InitBlendStates()
{

	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	ID3D11SamplerState* samplerState = NULL;
	pDevice->CreateSamplerState(&samplerDesc, &samplerState);

	pContext->PSSetSamplers(0, 1, &samplerState);

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	pDevice->CreateBlendState(&blendDesc, &bState[BLENDSTATE_NONE]);

	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	pDevice->CreateBlendState(&blendDesc, &bState[BLENDSTATE_ALFA]);


	// ========== 正确透明（预乘 Alpha，无黑边） ==========
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	pDevice->CreateBlendState(&blendDesc, &bState[BLENDSTATE_PREMULTIPLIED]);

	// ========== 加法 ==========
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	pDevice->CreateBlendState(&blendDesc, &bState[BLENDSTATE_ADD]);

	// ========== 减法 ==========
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_SUBTRACT;
	pDevice->CreateBlendState(&blendDesc, &bState[BLENDSTATE_SUB]);



	pContext->OMSetBlendState(bState[BLENDSTATE_ALFA], bFactor, 0xffffffff);



	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = FALSE;
	pDevice->CreateDepthStencilState(&depthStencilDesc, &g_DepthStateEnable);
	depthStencilDesc.DepthEnable = FALSE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	pDevice->CreateDepthStencilState(&depthStencilDesc, &g_DepthStateDisable);

	pContext->OMSetDepthStencilState(g_DepthStateDisable, NULL);

}


void Graphics::SetBlendState(BLENDSTATE blend)
{
	pContext->OMSetBlendState(bState[blend], bFactor, 0xffffffff);
}

void Graphics::Draw(ID3D11DepthStencilView* dsv, ID3D11RasterizerState* rs)
{
	
	//SetBlendState(BLENDSTATE::BLENDSTATE_PREMULTIPLIED);
	SetBlendState(BLENDSTATE::BLENDSTATE_ALFA);
	const UINT stride = sizeof(Vector);
	const UINT offset = 0u;
	pContext->IASetVertexBuffers(0u, 1u, g_pVertexBuffer.GetAddressOf(), &stride, &offset);
	pContext->IASetIndexBuffer(g_pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);
	pContext->IASetInputLayout(g_pInputLayout.Get());
	pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), dsv);
	pContext->RSSetState(rs ? rs : pRasterizerState.Get()); // ← 修正
	ResetConstantBuffer(pContext.Get());
	pContext->VSSetConstantBuffers(0u, 1u, g_pConstantBuffer.GetAddressOf());
	pContext->RSSetViewports(1u, &(*viewport));
	pContext->DrawIndexed(6, 0u, 0u);
}

void Graphics::EndFrame_3D()
{


	{

		float fNear = 0.1f;
		float fFar = 1000.0f;
		float fFov = 60.0f; // 比较平缓自然的透视
		float fAspectRatio = static_cast<float>(4069) / static_cast<float>(4069);
		float fFovRad = 1.0f / tanf(DirectX::XMConvertToRadians(fFov) * 0.5f);
		transform_Light_proj = {
			fAspectRatio * fFovRad,   0.0f,      0.0f ,    0.0f,
			0.0f,                    fFovRad,   0.0f ,    0.0f,
			0.0f,                    0.0f,      fFar / (fFar - fNear),    1.0f,
			0.0f,                    0.0f,      (-fFar * fNear) / (fFar - fNear),    0.0f,
		};

		/*float left = -50.0f;
		float right = 50.0f;
		float bottom = -50.0f;
		float top = 50.0f;
		float zn = 0.1f;
		float zf = 1000.0f;

		transform_Light_proj =
		{
			2.0f / (right - left), 0, 0, 0,
			0, 2.0f / (top - bottom), 0, 0,
			0, 0, 1.0f / (zf - zn), 0,
			-(right + left) / (right - left),
			-(top + bottom) / (top - bottom),
			-zn / (zf - zn),
			1
		};*/

	}

	SetBlendState(BLENDSTATE::BLENDSTATE_NONE);
	EnsureSceneLightTargets(SCREEN_WIDTH, SCREEN_HEIGHT);
	BeginSceneLightRender();
	RenderLightScene();
	EndSceneRender();

	float fNear = 0.1f;
	float fFar = 1000.0f;
	float fFov = 60.0f; // 比较平缓自然的透视
	float fAspectRatio = static_cast<float>(SCREEN_HEIGHT) / static_cast<float>(SCREEN_WIDTH);
	float fFovRad = 1.0f / tanf(DirectX::XMConvertToRadians(fFov) * 0.5f);
	transform_proj = {
		fAspectRatio * fFovRad,   0.0f,      0.0f ,    0.0f,
		0.0f,                    fFovRad,   0.0f ,    0.0f,
		0.0f,                    0.0f,      fFar / (fFar - fNear),    1.0f,
		0.0f,                    0.0f,      (-fFar * fNear) / (fFar - fNear),    0.0f,
	};

	SortTransparentObjects(WinCamera_3D.pos);
	SetBlendState(BLENDSTATE::BLENDSTATE_NONE);
	EnsureSceneOpaqueTargets(SCREEN_WIDTH, SCREEN_HEIGHT);
	// 渲染流程结构（按顺序执行）
	if (!object_3D_list_Darw_Opaque.empty())
	{
		BeginSceneOpaqueRender();
		RenderOpaqueScene();
		EndSceneRender();
	}

	//SetBlendState(BLENDSTATE::BLENDSTATE_PREMULTIPLIED);
	SetBlendState(BLENDSTATE::BLENDSTATE_ALFA);
	EnsureSceneTransparentTargets(SCREEN_WIDTH, SCREEN_HEIGHT);
	if (!object_3D_list_Darw_Transparent.empty())
	{
		// 半透明物体（Transparent）
		BeginSceneTransparentRender();
		RenderTransparentScene();
		EndSceneRender();
	}

}

// 更新相机常量缓冲（Unity 同款 OrbitCamera）
void Graphics::UpdateConstantBuffer(
	Camera_3D camera_3D,
	const DirectX::XMMATRIX& projMatrix)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource = {};
	HRESULT hr;
	hr = pContext->Map(camera_3D.g_cameraCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	if (FAILED(hr)) {
		MessageBox(NULL, L"Failed to map camera constant buffer.", L"Error", MB_OK | MB_ICONERROR);
		return;
	}

	Camera_3D::CameraBuffer* cb = reinterpret_cast<Camera_3D::CameraBuffer*>(mappedResource.pData);

	// ===== 1. 根据 yaw/pitch/distance 算 eye =====
	// yaw：左右旋转（绕Y）
	// pitch：上下旋转（绕X）
	DirectX::XMVECTOR quat = DirectX::XMQuaternionRotationRollPitchYaw(
		DirectX::XMConvertToRadians(camera_3D.pitch),
		DirectX::XMConvertToRadians(camera_3D.yaw),
		0.0f // 一般不考虑 roll
	);

	DirectX::XMMATRIX rotMatrix = DirectX::XMMatrixRotationQuaternion(quat);

	// target 是注视点，offset 是相机与目标的距离
	DirectX::XMVECTOR tgt = DirectX::XMLoadFloat3(&camera_3D.target);
	DirectX::XMVECTOR offset = DirectX::XMVector3Transform(
		DirectX::XMVectorSet(0.0f, 0.0f, -camera_3D.distance, 0.0f), rotMatrix
	);

	// ✅使用 Orbit 轨道逻辑自动计算位置
	DirectX::XMVECTOR  eye = DirectX::XMVectorAdd(tgt, offset);


	// ===== 2. Up 向量（相机的局部 Y） =====
	DirectX::XMVECTOR up = rotMatrix.r[1];

	// ===== 3. 生成 View / Proj =====
	view_imgui = DirectX::XMMatrixLookAtLH(eye, tgt, up);

	cb->view = DirectX::XMMatrixLookAtLH(eye, tgt, up);
	cb->proj = projMatrix;

	// ===== 4. 相机位置存到常量缓冲 =====
	DirectX::XMStoreFloat3(&cb->cameraPos, eye);
	cb->padding = 0.0f;


	pContext->Unmap(camera_3D.g_cameraCB.Get(), 0);

}

// 更新光源常量缓冲（Unity 同款 OrbitCamera）
void Graphics::UpdateConstantBuffer(
	const Light& light,
	const DirectX::XMMATRIX& projMatrix)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource = {};
	HRESULT hr = pContext->Map(
		light.pLightBuffer.Get(),
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&mappedResource
	);

	if (FAILED(hr)) {
		MessageBox(NULL, L"Failed to map Light constant buffer.", L"Error", MB_OK | MB_ICONERROR);
		return;
	}

	LightBufferData* lg = reinterpret_cast<LightBufferData*>(mappedResource.pData);

	// ✅ CPU → GPU 数据同步
	lg->lightDirection = light.data.direction;
	lg->intensity = light.data.intensity;
	lg->lightColor = light.data.color;
	lg->ambientFactor = 0.25f;
	lg->lightPos = light.data.position;
	lg->padding2 = 0.0f;

	pContext->Unmap(light.pLightBuffer.Get(), 0);

}

Object_3D::Bounds ComputeWorldBounds(const Object_3D * obj)
{
	using namespace DirectX;

	Object_3D::Bounds out{};

	// 1. 构建世界矩阵（S * R * T）
	XMMATRIX world =
		XMMatrixScaling(
			obj->Transform.Scale.x,
			obj->Transform.Scale.y,
			obj->Transform.Scale.z
		) *
		XMMatrixRotationRollPitchYaw(
			obj->Transform.Rotation.x,
			obj->Transform.Rotation.y,
			obj->Transform.Rotation.z
		) *
		XMMatrixTranslation(
			obj->Transform.Position.x,
			obj->Transform.Position.y,
			obj->Transform.Position.z
		);

	// 2. 变换包围盒中心点（局部 → 世界）
	XMVECTOR localCenter = XMVectorSet(
		obj->bound.center.x,
		obj->bound.center.y,
		obj->bound.center.z,
		1.0f
	);

	XMVECTOR worldCenterVec = XMVector3Transform(localCenter, world);
	XMStoreFloat3(&out.center, worldCenterVec);

	// 3. 处理包围盒尺寸（只考虑缩放，旋转不改变长度大小）
	out.size.x = obj->bound.size.x * obj->Transform.Scale.x;
	out.size.y = obj->bound.size.y * obj->Transform.Scale.y;
	out.size.z = obj->bound.size.z * obj->Transform.Scale.z;

	return out;
}

void Graphics::SortTransparentObjects(const DirectX::XMFLOAT3& cameraPos)
{
	std::sort(
		object_3D_list_Darw_Transparent.begin(),
		object_3D_list_Darw_Transparent.end(),
		[&](const Object_3D* a, const Object_3D* b)
		{
			Object_3D::Bounds wa = ComputeWorldBounds(a);
			Object_3D::Bounds wb = ComputeWorldBounds(b);

			//float dxA = wa.center.x - cameraPos.x;
			//float dyA = wa.center.y - cameraPos.y;
			//float dzA = wa.center.z - cameraPos.z;

			//float dxB = wb.center.x - cameraPos.x;
			//float dyB = wb.center.y - cameraPos.y;
			//float dzB = wb.center.z - cameraPos.z;

			float dxA = a->Transform.Position.x - cameraPos.x;
			float dyA = a->Transform.Position.y - cameraPos.y;
			float dzA = a->Transform.Position.z - cameraPos.z;

			float dxB = a->Transform.Position.x - cameraPos.x;
			float dyB = a->Transform.Position.y - cameraPos.y;
			float dzB = a->Transform.Position.z - cameraPos.z;

			float distA = dxA * dxA + dyA * dyA + dzA * dzA;
			float distB = dxB * dxB + dyB * dyB + dzB * dzB;

			// 远 → 近
			return distA > distB;
		}
	);
}


void Graphics::EndSceneRender()
{
	pContext->OMSetRenderTargets(1, pTarget.GetAddressOf(), nullptr);
	pContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}

// 更新物体常量缓冲
void Graphics::UpdateObjectConstantBuffer(
	Object_3D& object)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource = {};
	
	if (!object.pConstantBuffer) return;

	HRESULT hr = pContext->Map(
		object.pConstantBuffer.Get(),
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&mappedResource
	);

	if (FAILED(hr)) {
		MessageBoxA(nullptr, "Map 失败 (Object ConstantBuffer)", "错误", MB_OK | MB_ICONERROR);
		return;
	}

	// 拿到写指针
	MatrixBuffer_new* pData = reinterpret_cast<MatrixBuffer_new*>(mappedResource.pData);

	// === 生成世界矩阵 ===
	const Object_3D::Transform_3D& tf = object.Transform;

	DirectX::XMVECTOR scaling = DirectX::XMVectorSet(tf.Scale.x, tf.Scale.y, tf.Scale.z, 0.0f);
	DirectX::XMVECTOR rotationQuat = DirectX::XMQuaternionRotationRollPitchYaw(
		DirectX::XMConvertToRadians(tf.Rotation.x),
		DirectX::XMConvertToRadians(tf.Rotation.y),
		DirectX::XMConvertToRadians(tf.Rotation.z));
	DirectX::XMVECTOR translation = DirectX::XMVectorSet(tf.Position.x, tf.Position.y, tf.Position.z, 0.0f);

	// 组合矩阵
	pData->world = DirectX::XMMatrixAffineTransformation(
		scaling,
		DirectX::XMVectorZero(),
		rotationQuat,
		translation
	);
	/////////////////////////////////////////////////////////////////////////


	pContext->Unmap(object.pConstantBuffer.Get(), 0);
}

void Graphics::Init_3D(void)
{
	// 相机常量缓冲
	D3D11_BUFFER_DESC cbd = {};
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.ByteWidth = sizeof(Camera_3D::CameraBuffer);

	if (FAILED(Win->Gfx().pDevice->CreateBuffer(&cbd, nullptr, &WinCamera_3D.g_cameraCB)))
	{
		MessageBox(NULL, L"Failed to create camera constant buffer.", L"Error", MB_OK | MB_ICONERROR);
	}

	if (FAILED(Win->Gfx().pDevice->CreateBuffer(&cbd, nullptr, &WinLight_3D.g_cameraCB)))
	{
		MessageBox(NULL, L"Failed to create camera constant buffer.", L"Error", MB_OK | MB_ICONERROR);
	}

	LoadPS(pPixelShader_3D_Light, L"PS_Light.cso");
	LoadVS(pVertexShader_3D_Light, L"VS_Light.cso");
	////////////像素化
	LoadPS(pPixelShader_3D_Pixelation, L"PS_Pixelation.cso");


	light.type = LightType::Directional;

	light.data.type = (int)LightType::Directional;
	light.data.direction = { -0.4f, -1.0f, -0.3f };
	light.data.color = { 1.0f, 1.0f, 1.0f };
	light.data.intensity = 1.2f;
	light.data.enabled = 1;
	light.data.castShadow = 1;
	light.data.position = { 0,0,0 };
	light.data.range = 20;

	LightBufferData bufferData{};
	bufferData.lightDirection = light.data.direction;
	bufferData.intensity = light.data.intensity;
	bufferData.lightColor = light.data.color;
	bufferData.ambientFactor = 0.25f;

	bufferData.lightPos = light.data.position; // ✅新增
	bufferData.padding2 = 0.0f; // ✅对齐填充

	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.Usage = D3D11_USAGE_DYNAMIC;            // ✅允许 Map
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // ✅允许写
	cbd.ByteWidth = sizeof(LightBufferData);  // ✅48 字节，满足 16-byte 对齐

	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = &bufferData;

	HRESULT hr = pDevice->CreateBuffer(
		&cbd, &sd, &light.pLightBuffer
	);

	if (FAILED(hr))
	{
		MessageBox(NULL, L"Failed to create Light Constant Buffer", L"Error", MB_OK);
	}
}
#include "Graphics.h"
#include "system.h"


HRESULT Graphics::EnsureSceneTransparentTargets(UINT w, UINT h)
{
	// 先释放旧资源
	sceneTransparentSRV.Reset();
	sceneTransparentRTV.Reset();
	sceneTransparentTex.Reset();

	// 颜色纹理（可作为RTV+SRV）
	D3D11_TEXTURE2D_DESC td{};
	td.Width = w;
	td.Height = h;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;   // 常用、可作 SRV
	td.SampleDesc.Count = 1;// 多重采样
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	HRESULT hr = pDevice->CreateTexture2D(&td, nullptr, &sceneTransparentTex);
	if (FAILED(hr)) return hr;

	hr = pDevice->CreateRenderTargetView(sceneTransparentTex.Get(), nullptr, &sceneTransparentRTV);
	if (FAILED(hr)) return hr;


	hr = pDevice->CreateShaderResourceView(sceneTransparentTex.Get(), nullptr, &sceneTransparentSRV);
	if (FAILED(hr)) return hr;


	return S_OK;
}

void Graphics::BeginSceneTransparentRender()
{
	//只进行一次
	D3D11_DEPTH_STENCIL_DESC dsDesc{};
	dsDesc.DepthEnable = TRUE;                     // 深度测试开启
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;  // ❌ 禁止深度写入
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	ID3D11DepthStencilState* pDepthDisableWrite = nullptr;
	pDevice->CreateDepthStencilState(&dsDesc, &pDepthDisableWrite);

	pContext->OMSetDepthStencilState(pDepthDisableWrite, 1);

	// ⭐ 立刻释放，防止堆积 ⭐
	pDepthDisableWrite->Release();


	float clear[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	//pContext->ClearDepthStencilView(sceneDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	pContext->OMSetRenderTargets(1, sceneTransparentRTV.GetAddressOf(), sceneDSV.Get());
	pContext->ClearRenderTargetView(sceneTransparentRTV.Get(), clear);
	//pContext->ClearDepthStencilView(sceneDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);


}

bool Graphics::RenderTransparentScene(void)
{

	// === 遍历所有场景对象 ===
	for (auto& obj : object_3D_list_Darw_Transparent)
	{
		// 更新物体自身常量缓冲 (世界矩阵)
		UpdateObjectConstantBuffer(*obj);

		// 绑定物体常量缓冲到 VS slot0
		pContext->VSSetConstantBuffers(0, 1, obj->pConstantBuffer.GetAddressOf());
		pContext->VSSetConstantBuffers(1, 1, WinCamera_3D.g_cameraCB.GetAddressOf());

		pContext->PSSetConstantBuffers(2, 1, light.pLightBuffer.GetAddressOf());
		// 绑定到 VS 和 PS 的常量槽 (slot = 1)
		pContext->VSSetConstantBuffers(3, 1, WinCamera_3D.g_cameraCB.GetAddressOf());
		// === 遍历该对象的所有 Mesh ===

		//pContext->OMSetDepthStencilState(pMainDepthStencilState.Get(), 1); // ✅ 默认状态

		// 绑定顶点缓冲区
		const UINT stride = sizeof(VertexStatic3D);
		const UINT offset = 0u;
		pContext->IASetVertexBuffers(0u, 1u, obj->pVertexBuffer.GetAddressOf(), &stride, &offset);

		// 绑定索引缓冲区
		pContext->IASetIndexBuffer(
			obj->pIndexBuffer.Get(),
			DXGI_FORMAT_R32_UINT,
			0u
		);


		// === 材质绑定 ===
		// 顶点 Shader
		if (!obj->pVertexShader) return false;
		pContext->VSSetShader(obj->pVertexShader.Get(), nullptr, 0u);
		// 像素 Shader

		if (!obj->pPixelShader) return false;
		pContext->PSSetShader(obj->pPixelShader.Get(), nullptr, 0u);

		// 输入布局
		pContext->IASetInputLayout(
			obj->pInputLayout ? obj->pInputLayout.Get() : pInputLayout_3D.Get()
		);


		// 渲染状态

		// 光栅化状态
		if (!obj->pRasterizerState) return false;

		pContext->RSSetState(obj->pRasterizerState.Get());
		// 视口（更新宽高）
		pContext->RSSetViewports(1u, &(*viewport));


		// 漫反射贴图
		for (int i = 0;i < obj->pTextures.size();i++)
		{
			pContext->PSSetShaderResources(i + 1, 1, obj->pTextures[i].GetAddressOf());
		}


		//// === 保证深度/模板状态正常 ===
		//if (pMainDepthStencilState)
		//	pContext->OMSetDepthStencilState(pMainDepthStencilState.Get(), 1);

		// === 执行绘制 ===
		pContext->DrawIndexed(static_cast<UINT>(obj->indicesCount), 0u, 0u);
	}





	return true;
}
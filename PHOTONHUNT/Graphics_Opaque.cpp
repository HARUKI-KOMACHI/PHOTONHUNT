#include "Graphics.h"
#include "system.h"

// Graphics.cpp
HRESULT Graphics::EnsureSceneOpaqueTargets(UINT w, UINT h)
{
	if (w == 0 || h == 0) return S_FALSE;
	// 先释放旧资源
	sceneOpaqueSRV.Reset();
	sceneOpaqueRTV.Reset();
	sceneOpaqueTex.Reset();
	sceneDSV.Reset();
	sceneDepth.Reset();

	// 颜色纹理（可作为RTV+SRV）
	D3D11_TEXTURE2D_DESC td{};
	td.Width = w;
	td.Height = h;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;   // 常用、可作 SRV
	//td.Format = DXGI_FORMAT_R32_FLOAT;
	td.SampleDesc.Count = 1;// 多重采样
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	HRESULT hr = pDevice->CreateTexture2D(&td, nullptr, &sceneOpaqueTex);
	if (FAILED(hr)) return hr;

	hr = pDevice->CreateRenderTargetView(sceneOpaqueTex.Get(), nullptr, &sceneOpaqueRTV);
	if (FAILED(hr)) return hr;


	hr = pDevice->CreateShaderResourceView(sceneOpaqueTex.Get(), nullptr, &sceneOpaqueSRV);
	if (FAILED(hr)) return hr;

	D3D11_TEXTURE2D_DESC dd = td;
	dd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	hr = pDevice->CreateTexture2D(&dd, nullptr, &sceneDepth);
	if (FAILED(hr)) return hr;
	hr = pDevice->CreateDepthStencilView(sceneDepth.Get(), nullptr, &sceneDSV);
	if (FAILED(hr)) return hr;


	return S_OK;
}

// Graphics.cpp
void Graphics::BeginSceneOpaqueRender()
{
	float clear[4] = { 0.2f, 0.3f, 0.4f, 1.0f };
	pContext->OMSetRenderTargets(1, sceneOpaqueRTV.GetAddressOf(), sceneDSV.Get());
	pContext->ClearRenderTargetView(sceneOpaqueRTV.Get(), clear);
	pContext->ClearDepthStencilView(sceneDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

}

bool Graphics::RenderOpaqueScene(void)
{

	// === 绑定相机常量缓冲 ===
	UpdateConstantBuffer(WinCamera_3D,transform_proj);



	// === 遍历所有场景对象 ===
	for (auto& obj : object_3D_list_Darw_Opaque)
	{
		// 更新物体自身常量缓冲 (世界矩阵)
		UpdateObjectConstantBuffer(*obj);

		// 绑定物体常量缓冲到 VS slot0
		pContext->VSSetConstantBuffers(0, 1, obj->pConstantBuffer.GetAddressOf());
		pContext->VSSetConstantBuffers(1, 1, WinCamera_3D.g_cameraCB.GetAddressOf());
		UpdateConstantBuffer(light, transform_Light_proj);
		pContext->PSSetConstantBuffers(2, 1, light.pLightBuffer.GetAddressOf());
		pContext->VSSetConstantBuffers(3, 1, WinLight_3D.g_cameraCB.GetAddressOf());

		pContext->OMSetDepthStencilState(pMainDepthStencilState.Get(), 1); // ✅ 默认状态

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

		//pContext->PSSetShaderResources(0, 1, obj->pTextures[i].GetAddressOf());
		pContext->PSSetShaderResources(0, 1, sceneLightDepthSRV.GetAddressOf());
		// 漫反射贴图
		for (int i = 0;i < obj->pTextures.size();i++)
		{
			pContext->PSSetShaderResources(i + 1, 1, obj->pTextures[i].GetAddressOf());
		}
		// === 保证深度/模板状态正常 ===
		if (pMainDepthStencilState)
			pContext->OMSetDepthStencilState(pMainDepthStencilState.Get(), 1);

		if (obj->BoneInfos.size() != 0)
		{
			//Win->Gfx().pContext->VSSetConstantBuffers(4, 1, obj->skinCBuf.GetAddressOf());
			obj->UploadSkinningCB();
		}
		// === 执行绘制 ===
		pContext->DrawIndexed(static_cast<UINT>(obj->indicesCount), 0u, 0u);
			
	}

	return true;
}





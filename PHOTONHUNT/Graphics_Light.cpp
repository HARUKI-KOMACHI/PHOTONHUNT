


#include "Graphics.h"
#include "system.h"


// Graphics.cpp
HRESULT Graphics::EnsureSceneLightTargets(UINT w, UINT h)
{
	if (w == 0 || h == 0) return S_FALSE;
	// 光照
	sceneLightDepth.Reset();
	sceneLightRTV.Reset();
	sceneLightDepthSRV.Reset();

	sceneDSV.Reset();
	sceneDepth.Reset();

	// 颜色纹理（可作为RTV+SRV）
	D3D11_TEXTURE2D_DESC td{};
	td.Width = w;
	td.Height = h;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R32_FLOAT;
	td.SampleDesc.Count = 1;// 多重采样
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	//光照深度

	HRESULT hr = pDevice->CreateTexture2D(&td, nullptr, &sceneLightDepth);
	if (FAILED(hr)) return hr;

	hr = pDevice->CreateRenderTargetView(sceneLightDepth.Get(), nullptr, &sceneLightRTV);
	if (FAILED(hr)) return hr;


	hr = pDevice->CreateShaderResourceView(sceneLightDepth.Get(), nullptr, &sceneLightDepthSRV);
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
void Graphics::BeginSceneLightRender(void)
{
	float clear[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	pContext->OMSetRenderTargets(1, sceneLightRTV.GetAddressOf(), sceneDSV.Get());
	pContext->ClearRenderTargetView(sceneLightRTV.Get(), clear);
	pContext->ClearDepthStencilView(sceneDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

}

bool Graphics::RenderLightScene(void)
{

	// === 绑定相机常量缓冲 ===
	UpdateConstantBuffer(WinLight_3D, transform_Light_proj);

	light.data.position = WinLight_3D.target;

	// ✅ 从光源坐标计算方向：指向原点
	DirectX::XMFLOAT3 pos = light.data.position;
	DirectX::XMVECTOR posVec = DirectX::XMLoadFloat3(&pos);
	DirectX::XMVECTOR originVec = DirectX::XMVectorZero();
	DirectX::XMVECTOR dirVec = DirectX::XMVector3Normalize(
		DirectX::XMVectorSubtract(originVec, posVec)
	);

	// 存回 light.data.direction
	DirectX::XMFLOAT3 lightdir;
	DirectX::XMStoreFloat3(&lightdir, dirVec);
	light.data.direction = lightdir;


	UpdateConstantBuffer(light, transform_Light_proj);
	// ✅只绑定到 VS（b2：LightBuffer）
	pContext->VSSetConstantBuffers(2, 1,
		light.pLightBuffer.GetAddressOf());


	// === 遍历所有场景对象 ===
	for (auto& obj : object_3D_list_Darw_Opaque)
	{
		// 更新物体自身常量缓冲 (世界矩阵)
		UpdateObjectConstantBuffer(*obj);

		// 绑定物体常量缓冲到 VS slot0
		pContext->VSSetConstantBuffers(0, 1, obj->pConstantBuffer.GetAddressOf());
		pContext->VSSetConstantBuffers(1, 1, WinLight_3D.g_cameraCB.GetAddressOf());

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
		pContext->VSSetShader(pVertexShader_3D_Light.Get(), nullptr, 0u);
		// 像素 Shader
		pContext->PSSetShader(pPixelShader_3D_Light.Get(), nullptr, 0u);



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
			pContext->PSSetShaderResources(i, 1, obj->pTextures[i].GetAddressOf());
		}
		// === 保证深度/模板状态正常 ===
		if (pMainDepthStencilState)
			pContext->OMSetDepthStencilState(pMainDepthStencilState.Get(), 1);

		// === 执行绘制 ===
		pContext->DrawIndexed(static_cast<UINT>(obj->indicesCount), 0u, 0u);

	}


	return true;
}
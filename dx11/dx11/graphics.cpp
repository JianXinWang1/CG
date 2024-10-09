#include "graphics.h"
#include "d3dcompiler.h"
#include <xutility>
#include "WICTextureLoader11.h"
#include <string>
#include <iostream>
#include "debug.h"
using namespace DirectX;
using namespace std;

Graphics::Graphics(HWND hWnd) {
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
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

	UINT swapCreateFlags = 0u;
	D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&sd,
		&pSwapChain,
		&pDevice,
		nullptr,
		&pDeviceContext
	);
	ID3D11Resource* pBackBuffer = nullptr;
	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(& pBackBuffer));
	pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &targetView);
	pBackBuffer->Release();

	vertices[0] = { -0.5, 0.5, 0.0, 1.0};
	vertices[1] = { -0.5,  0.5, 0.0, 0.0};
	vertices[2] = { 0.5, 0.5, 1.0, 0.0 };
	vertices[3] = { 0.5, -0.5, 1.0, 1.0};
	
	tr = {
		{
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		}
	};

	index[0] = { 0, 1, 2 };
	index[1] = { 0, 2, 3 };
}


void Graphics::SwapBuffer() {
	pSwapChain->Present(1u, 0u);
}

void Graphics::ClearBuffer(float r, float g, float b) {
	const float rgb[] = { r, g, b, 1.0 };
	pDeviceContext->ClearRenderTargetView(targetView.Get(), rgb);
}

void Graphics::CreatBuffer() {
	// 顶点属性描述
	D3D11_BUFFER_DESC verticsDesc;
	verticsDesc.ByteWidth = sizeof(vertices); // 总字节数
	verticsDesc.Usage = D3D11_USAGE_DEFAULT; // 默认
	verticsDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // API推荐 
	verticsDesc.CPUAccessFlags = 0; // CPU 的读写权限
	verticsDesc.MiscFlags = 0;
	verticsDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA resourceData;
	resourceData.pSysMem = vertices;
	resourceData.SysMemPitch = 0; // 纹理相关先设置为0
	resourceData.SysMemSlicePitch = 0; // 纹理相关先设置为0

	// 顶点缓冲
	DEBUG = pDevice->CreateBuffer(&verticsDesc, &resourceData, &verticesBuffer);

	D3D11_BUFFER_DESC indexDesc = {};
	indexDesc.ByteWidth = sizeof(index); // 字节数
	// 将 usage 设为 D3D11_USAGE_IMMUTABLE  D3D11_USAGE_DEFAULT 可行
	indexDesc.Usage = D3D11_USAGE_IMMUTABLE; // 资源的使用，gpu和cpu 的读写权限 
	indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER; // 标识如何将资源绑定到 pipeline 
	indexDesc.CPUAccessFlags = 0; // CPU 的读写权限
	indexDesc.MiscFlags = 0;
	indexDesc.StructureByteStride = 0;

	resourceData.pSysMem = index;
	resourceData.SysMemPitch = 0; // 纹理相关先设置为0
	resourceData.SysMemSlicePitch = 0; // 纹理相关先设置为0

	DEBUG = pDevice->CreateBuffer(&indexDesc, &resourceData, indexBuffer.GetAddressOf());
	
	// 设置所引缓存
	pDeviceContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	

	// 顶点缓冲读取描述
	UINT strider = sizeof(Vertex);
	UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(
		0,
		1,
		verticesBuffer.GetAddressOf(),// 顶点缓存
		&strider,		// 每组数据的字节数
		&offset);		// 偏移量

	std::wstring path = L"res\\image\\wjx.png";
	DEBUG = CreateWICTextureFromFile(pDevice.Get(),
		path.c_str(),
		&inputResource,
		&shaderResourceView);
	Debug::DebugHr(DEBUG);
	pDeviceContext->PSSetShaderResources(0, 1, &shaderResourceView);

	ID3D11SamplerState* sampler;
	D3D11_SAMPLER_DESC sampleDesc = {};
	sampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // 平铺整数个
	sampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampleDesc.MinLOD = 0;
	sampleDesc.MaxLOD = D3D11_FLOAT32_MAX;
	DEBUG = pDevice->CreateSamplerState(&sampleDesc, &sampler);
	
	pDeviceContext->PSSetSamplers(0, 1, &sampler);


	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	// 顶点着色器
	D3DReadFileToBlob(L"VertexShader.cso", &pBlob);
	DEBUG = pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader);
	Debug::DebugHr(DEBUG);
	pDeviceContext->VSSetShader(pVertexShader.Get(), nullptr, 0);

	// layout连接顶点着色器
	DEBUG = pDevice->CreateInputLayout(layout,
		size(layout),
		pBlob->GetBufferPointer(), // 该 shader 有 layout 中定义的 SemanticName
		pBlob->GetBufferSize(),
		&inputLayout);
	pDeviceContext->IASetInputLayout(inputLayout.Get());

	// 片段着色器
	D3DReadFileToBlob(L"PixelShader.cso", &pBlob);

	pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader);
	pDeviceContext->PSSetShader(pPixelShader.Get(), nullptr, 0);
}

void Graphics::CreatConstBuffer() {
	D3D11_BUFFER_DESC transformDesc;
	transformDesc.ByteWidth = sizeof(tr);
	transformDesc.Usage = D3D11_USAGE_DYNAMIC;
	transformDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	transformDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // CPU 的读写权限
	transformDesc.MiscFlags = 0;
	transformDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA transformResource;
	transformResource.pSysMem = &tr;

	pDevice->CreateBuffer(&transformDesc, &transformResource, &transformBuffer);

	pDeviceContext->VSSetConstantBuffers(0, 1, transformBuffer.GetAddressOf());
}

void Graphics::DrawTriangle(float A, float S, float D, float W, float Q, float E) {
	// transform
	tr.element[0][0] = cos((Q + E));
	tr.element[0][1] = sin(Q + E);
	tr.element[1][0] = -sin(Q + E);
	tr.element[1][1] = cos((Q + E));
	tr.element[0][3] = (A + D);
	tr.element[1][3] =(S + W);

	CreatConstBuffer();
	// 指定图元拓扑结构
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 设置渲染目标
	pDeviceContext->OMSetRenderTargets(1, targetView.GetAddressOf(), NULL);

	// 设置视口
	D3D11_VIEWPORT viewPort;
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.Width = 640;
	viewPort.Height = 480;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;
	pDeviceContext->RSSetViewports(1, &viewPort);

	// 绘制
	pDeviceContext->Draw(size(vertices), 0);
}

void Graphics::CreatTexBuffer() {

}
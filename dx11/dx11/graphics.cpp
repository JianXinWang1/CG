#include "graphics.h"
#include "d3dcompiler.h"
#include <xutility>
#include <iostream>
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

	vertices[0] = { 0.0, 0.5, 255, 0, 0 };
	vertices[1] = { 0.5,  0.0, 0, 255, 0 };
	vertices[2] = { -0.5, 0.0, 0, 0, 255 };

	tr = {
		{
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		}
	};
}


void Graphics::SwapBuffer() {
	pSwapChain->Present(1u, 0u);
}

void Graphics::ClearBuffer(float r, float g, float b) {
	const float rgb[] = { r, g, b, 1.0 };
	pDeviceContext->ClearRenderTargetView(targetView.Get(), rgb);
}

void Graphics::CreatBuffer() {
	// ������������
	D3D11_BUFFER_DESC verticsDesc;
	verticsDesc.ByteWidth = sizeof(vertices); // ���ֽ���
	verticsDesc.Usage = D3D11_USAGE_DEFAULT; // Ĭ��
	verticsDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // API�Ƽ� 
	verticsDesc.CPUAccessFlags = 0; // CPU �Ķ�дȨ��
	verticsDesc.MiscFlags = 0;
	verticsDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA resourceData;
	resourceData.pSysMem = vertices;
	resourceData.SysMemPitch = 0; // �������������Ϊ0
	resourceData.SysMemSlicePitch = 0; // �������������Ϊ0

	// ���㻺��
	pDevice->CreateBuffer(&verticsDesc, &resourceData, &verticesBuffer);

	// ���㻺���ȡ����
	UINT strider = sizeof(Vertex);
	UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(
		0,
		1,
		verticesBuffer.GetAddressOf(),// ���㻺��
		&strider,		// ÿ�����ݵ��ֽ���
		&offset);		// ƫ����

	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "Color", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	// ������ɫ��
	D3DReadFileToBlob(L"VertexShader.cso", &pBlob);
	pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader);
	pDeviceContext->VSSetShader(pVertexShader.Get(), nullptr, 0);

	// layout���Ӷ�����ɫ��
	pDevice->CreateInputLayout(layout,
		size(layout),
		pBlob->GetBufferPointer(), // �� shader �� layout �ж���� SemanticName
		pBlob->GetBufferSize(),
		&inputLayout);
	pDeviceContext->IASetInputLayout(inputLayout.Get());

	// Ƭ����ɫ��
	D3DReadFileToBlob(L"PixelShader.cso", &pBlob);

	pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader);
	pDeviceContext->PSSetShader(pPixelShader.Get(), nullptr, 0);
}

void Graphics::CreatConstBuffer() {
	D3D11_BUFFER_DESC transformDesc;
	transformDesc.ByteWidth = sizeof(tr);
	transformDesc.Usage = D3D11_USAGE_DYNAMIC;
	transformDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	transformDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // CPU �Ķ�дȨ��
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
	// ָ��ͼԪ���˽ṹ
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ������ȾĿ��
	pDeviceContext->OMSetRenderTargets(1, targetView.GetAddressOf(), NULL);

	// �����ӿ�
	D3D11_VIEWPORT viewPort;
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.Width = 640;
	viewPort.Height = 480;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;
	pDeviceContext->RSSetViewports(1, &viewPort);

	// ����
	pDeviceContext->Draw(size(vertices), 0);
}
#pragma once
#include<d3d11.h>
#include <wrl.h>

using namespace Microsoft::WRL;

struct Vertex
{
	float x;
	float y;
	float u;
	float v;
};

struct Transform {
	float element[4][4];
};

struct Index {
	UINT a;
	UINT b;
	UINT c;
};

class Graphics {
public:
	Graphics(HWND hWnd);
	void SwapBuffer();
	void ClearBuffer(float r, float g, float b);
	void DrawTriangle(float A, float S, float D, float W, float Q, float E);
	void CreatBuffer();
	void CreatConstBuffer();
	void CreatTexBuffer();
private:
	ComPtr<ID3D11Device> pDevice;
	ComPtr<ID3D11DeviceContext> pDeviceContext;
	ComPtr<IDXGISwapChain> pSwapChain;
	DXGI_SWAP_CHAIN_DESC sd;
	ComPtr<ID3D11RenderTargetView> targetView;
	Vertex vertices[4];
	Transform tr;
	ComPtr<ID3D11Buffer> verticesBuffer;
	ComPtr<ID3D11Buffer> transformBuffer;
	ComPtr<ID3D11Buffer> indexBuffer;
	ComPtr<ID3D11PixelShader> pPixelShader;
	ComPtr<ID3D11InputLayout> inputLayout;
	ComPtr<ID3DBlob> pBlob;
	ComPtr<ID3D11VertexShader> pVertexShader;
	ComPtr<ID3D11Resource> inputResource;
	ComPtr<ID3D11ShaderResourceView> shaderResourceView;
	Index index[2];
	HRESULT DEBUG;

};
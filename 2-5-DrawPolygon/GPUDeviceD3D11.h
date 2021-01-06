#pragma once
#ifndef GPU_DEVICE_D3D11_H_INCLUDED
#define GPU_DEVICE_D3D11_H_INCLUDED

#include <cstdint>
#include <vector>
#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <DirectXMath.h>

class GPUDeviceD3D11
{
public:
	bool initialize(HWND hWnd);
	bool render();
private:
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	bool createDevice();
	bool retrieveDXGIFactory();

	// Input Assembler (IA)
	bool createInputLayout(const void * p_bytecode, size_t bytecode_length);
	bool createVertexBuffer();

	// Vertex Shader (VS)
	bool createVertexShader(const void * p_bytecode, size_t bytecode_length);

	// Rasterizer Stage (RS)

	// Pixel Shader (PS)
	bool createPixelShader(const void * p_bytecode, size_t bytecode_length);

	// Output Merger (OM)
	bool createSwapChain();
	bool createRTV();
	bool setupGraphicsPipeline();

private:
	HWND mhWnd = nullptr;
	uint32_t mWidth = 0;
	uint32_t mHeight = 0;

	D3D_FEATURE_LEVEL mFeatureLevel = D3D_FEATURE_LEVEL_11_1;
	ComPtr<ID3D11Device> mpDevice;
	ComPtr<ID3D11DeviceContext> mpImmediateContext;

	ComPtr<IDXGIFactory2> mpDXGIFactory;

	// Input Assembler (IA)
	ComPtr<ID3D11InputLayout> mpInputLayout;

	ComPtr<ID3D11Buffer> mpVertexBuffer;
	uint32_t mVertexStride = 0;
	uint32_t mVertexOffset = 0;

	// Vertex Shader (VS)
	ComPtr<ID3D11VertexShader> mpVertexShader;

	// Rasterizer Stage (RS)
	D3D11_VIEWPORT mViewport;

	// Pixel Shader (PS)
	ComPtr<ID3D11PixelShader> mpPixelShader;

	// Output Merger (OM)
	DXGI_FORMAT mSwapChainBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	uint32_t mSwapChainBufferCount = 2;
	ComPtr<IDXGISwapChain1> mpSwapChain;

	ComPtr<ID3D11RenderTargetView> mpRTV;

	float mClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
};

#endif // GPU_DEVICE_D3D11_H_INCLUDED

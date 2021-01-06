#pragma once
#ifndef GPU_DEVICE_D3D11_H_INCLUDED
#define GPU_DEVICE_D3D11_H_INCLUDED

#include <cstdint>
#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

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

	DXGI_FORMAT mSwapChainBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	uint32_t mSwapChainBufferCount = 2;
	ComPtr<IDXGISwapChain1> mpSwapChain;

	ComPtr<ID3D11RenderTargetView> mpRTV;

	float mClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
};

#endif // GPU_DEVICE_D3D11_H_INCLUDED

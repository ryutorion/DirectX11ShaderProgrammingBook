﻿#pragma once
#ifndef GPU_DEVICE_D3D11_H_INCLUDED
#define GPU_DEVICE_D3D11_H_INCLUDED

#include <cstdint>
#include <vector>
#include <string>
#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <DirectXMath.h>

class GPUDeviceD3D11
{
public:
	~GPUDeviceD3D11();

	bool initialize(HWND hWnd);
	bool render();
private:
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	bool createDevice();
	bool retrieveDXGIFactory();

	bool loadMesh();

	// Input Assembler (IA)
	bool createInputLayout(const void * p_bytecode, size_t bytecode_length);
	bool createVertexBuffer();
	bool createIndexBuffer();

	// Vertex Shader (VS)
	bool createVertexShader(const void * p_bytecode, size_t bytecode_length);
	bool createVSConstantBuffer();

	// Rasterizer Stage (RS)
	bool createRasterizerState();

	// Pixel Shader (PS)
	bool createPixelShader(const void * p_bytecode, size_t bytecode_length);
	bool createSamplerState();

	// Output Merger (OM)
	bool createSwapChain();
	bool createRTV();
	bool createBlendStates();

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

	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 uv;
	};
	std::vector<Vertex> mVertices;
	std::vector<uint32_t> mIndices;

	ComPtr<ID3D11Buffer> mpVertexBuffer;
	uint32_t mVertexStride = 0;
	uint32_t mVertexOffset = 0;

	ComPtr<ID3D11Buffer> mpIndexBuffer;

	// Vertex Shader (VS)
	ComPtr<ID3D11VertexShader> mpVertexShader;

	ComPtr<ID3D11Buffer> mpVSConstantBuffer;

	// Rasterizer Stage (RS)
	D3D11_VIEWPORT mViewport;
	ComPtr<ID3D11RasterizerState> mpRS;

	// Pixel Shader (PS)
	ComPtr<ID3D11PixelShader> mpPixelShader;
	ComPtr<ID3D11ShaderResourceView> mpTextureSRV;
	ComPtr<ID3D11SamplerState> mpSamplerState;

	// Output Merger (OM)
	DXGI_FORMAT mSwapChainBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	uint32_t mSwapChainBufferCount = 2;
	ComPtr<IDXGISwapChain1> mpSwapChain;

	ComPtr<ID3D11RenderTargetView> mpRTV;
	float mClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	ComPtr<ID3D11BlendState> mpEarthBS;
	ComPtr<ID3D11BlendState> mpCloudBS;
};

#endif // GPU_DEVICE_D3D11_H_INCLUDED

#include "GPUDeviceD3D11.h"
#include <d3dcompiler.h>

namespace
{
	template <class T, size_t N>
	constexpr uint32_t countof(const T (&)[N]) noexcept
	{
		return static_cast<uint32_t>(N);
	}

	template <class T>
	concept sizable = requires(const T & s)
	{
		s.size();
	};

	template <sizable T>
	uint32_t countof(const T & s) noexcept
	{
		return static_cast<uint32_t>(s.size());
	}

	template <sizable T>
	uint32_t bytesof(const T & s) noexcept
	{
		return static_cast<uint32_t>(sizeof(s[0]) * s.size());
	}

	bool loadShader(
		const wchar_t * p_file_name,
		const char * p_entry_point,
		const char * p_target,
		Microsoft::WRL::ComPtr<ID3DBlob> & p_shader_blob
	)
	{
		uint32_t compile_flags = D3DCOMPILE_ENABLE_STRICTNESS;
		compile_flags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
#if _DEBUG
		compile_flags |= D3DCOMPILE_DEBUG;
		compile_flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		Microsoft::WRL::ComPtr<ID3DBlob> p_error_blob;

		HRESULT hr = D3DCompileFromFile(
			p_file_name,
			nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			p_entry_point,
			p_target,
			compile_flags,
			0,
			&p_shader_blob,
			&p_error_blob
		);
		if(FAILED(hr))
		{
			OutputDebugString(
				reinterpret_cast<const char *>(p_error_blob->GetBufferPointer())
			);
			return false;
		}

		return true;
	}

	struct Vertex
	{
		DirectX::XMFLOAT4 position;
		DirectX::XMFLOAT4 color;
	};

	std::vector<Vertex> vertices
	{
		{ DirectX::XMFLOAT4(150.0f - 150.0f, -50.0f + 150.0f, 0.5f * 150.0f, 150.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ DirectX::XMFLOAT4(250.0f - 150.0f, -250.0f + 150.0f, 0.5f * 150.0f, 150.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ DirectX::XMFLOAT4( 50.0f - 150.0f, -250.0f + 150.0f, 0.5f * 150.0f, 150.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
	};
}

bool GPUDeviceD3D11::initialize(HWND hWnd)
{
	mhWnd = hWnd;

	RECT client_rect;
	GetClientRect(hWnd, &client_rect);

	mWidth = static_cast<uint32_t>(client_rect.right - client_rect.left);
	mHeight = static_cast<uint32_t>(client_rect.bottom - client_rect.top);

	if(!createDevice())
	{
		return false;
	}

	if(!retrieveDXGIFactory())
	{
		return false;
	}

	ComPtr<ID3DBlob> p_vertex_shader_blob;
	if(!loadShader(L"shaders.hlsl", "VS", "vs_5_0", p_vertex_shader_blob))
	{
		return false;
	}

	// Input Assembler (IA)
	if(!createInputLayout(
		p_vertex_shader_blob->GetBufferPointer(),
		p_vertex_shader_blob->GetBufferSize()
	))
	{
		return false;
	}

	if(!createVertexBuffer())
	{
		return false;
	}

	// Vertex Shader (VS)
	if(!createVertexShader(
		p_vertex_shader_blob->GetBufferPointer(),
		p_vertex_shader_blob->GetBufferSize()
	))
	{
		return false;
	}

	// Rasterizer Stage (RS)
	mViewport.TopLeftX = 0.0f;
	mViewport.TopLeftY = 0.0f;
	mViewport.Width = static_cast<float>(mWidth);
	mViewport.Height = static_cast<float>(mHeight);
	mViewport.MinDepth = D3D11_MIN_DEPTH;
	mViewport.MaxDepth = D3D11_MAX_DEPTH;

	// Pixel Shader (PS)
	ComPtr<ID3DBlob> p_pixel_shader_blob;
	if(!loadShader(L"shaders.hlsl", "PS", "ps_5_0", p_pixel_shader_blob))
	{
		return false;
	}

	if(!createPixelShader(
		p_pixel_shader_blob->GetBufferPointer(),
		p_pixel_shader_blob->GetBufferSize()
	))
	{
		return false;
	}

	// Output Merger (OM)
	if(!createSwapChain())
	{
		return false;
	}

	if(!createRTV())
	{
		return false;
	}

	return true;
}

bool GPUDeviceD3D11::render()
{
	if(!setupGraphicsPipeline())
	{
		return false;
	}

	mpImmediateContext->ClearRenderTargetView(mpRTV.Get(), mClearColor);

	mpImmediateContext->Draw(countof(vertices), 0);

	uint32_t present_flags = 0;
	mpSwapChain->Present(0, present_flags);

	return true;
}

bool GPUDeviceD3D11::createDevice()
{
	uint32_t device_creation_flags = 0;
#if _DEBUG
	device_creation_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL feature_levels[]
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	ComPtr<ID3D11Device> p_device;
	ComPtr<ID3D11DeviceContext> p_immediate_context;
	HRESULT hr = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		device_creation_flags,
		feature_levels,
		countof(feature_levels),
		D3D11_SDK_VERSION,
		&p_device,
		&mFeatureLevel,
		&p_immediate_context
	);
	if(FAILED(hr))
	{
		return false;
	}

	if(FAILED(p_device.As(&mpDevice)))
	{
		return false;
	}

	if(FAILED(p_immediate_context.As(&mpImmediateContext)))
	{
		return false;
	}

	return true;
}

bool GPUDeviceD3D11::retrieveDXGIFactory()
{
	ComPtr<IDXGIDevice> p_dxgi_device;
	if(FAILED(mpDevice.As(&p_dxgi_device)))
	{
		return false;
	}

	ComPtr<IDXGIAdapter> p_dxgi_adapter;
	if(FAILED(p_dxgi_device->GetAdapter(&p_dxgi_adapter)))
	{
		return false;
	}

	if(FAILED(p_dxgi_adapter->GetParent(IID_PPV_ARGS(&mpDXGIFactory))))
	{
		return false;
	}

	return true;
}

bool GPUDeviceD3D11::createInputLayout(const void * p_bytecode, size_t bytecode_length)
{
	D3D11_INPUT_ELEMENT_DESC input_element_descs[]
	{
		{
			.SemanticName = "POSITION",
			.SemanticIndex = 0,
			.Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
			.InputSlot = 0,
			.AlignedByteOffset = offsetof(Vertex, position),
			.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0
		},
		{
			.SemanticName = "COLOR",
			.SemanticIndex = 0,
			.Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
			.InputSlot = 0,
			.AlignedByteOffset = offsetof(Vertex, color),
			.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0
		},
	};

	HRESULT hr = mpDevice->CreateInputLayout(
		input_element_descs,
		countof(input_element_descs),
		p_bytecode,
		bytecode_length,
		&mpInputLayout
	);
	if(FAILED(hr))
	{
		return false;
	}

	return true;
}

bool GPUDeviceD3D11::createVertexBuffer()
{
	D3D11_BUFFER_DESC buffer_desc
	{
		.ByteWidth = bytesof(vertices),
		.Usage = D3D11_USAGE_IMMUTABLE,
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
		.CPUAccessFlags = 0,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};

	D3D11_SUBRESOURCE_DATA subresource_data
	{
		.pSysMem = vertices.data(),
		.SysMemPitch = 0,
		.SysMemSlicePitch = 0
	};

	HRESULT hr = mpDevice->CreateBuffer(
		&buffer_desc,
		&subresource_data,
		&mpVertexBuffer
	);
	if(FAILED(hr))
	{
		return false;
	}

	mVertexStride = sizeof(Vertex);
	mVertexOffset = 0;

	return true;
}

bool GPUDeviceD3D11::createVertexShader(const void * p_bytecode, size_t bytecode_length)
{
	HRESULT hr = mpDevice->CreateVertexShader(
		p_bytecode,
		bytecode_length,
		nullptr,
		&mpVertexShader
	);
	if(FAILED(hr))
	{
		return false;
	}

	return true;
}

bool GPUDeviceD3D11::createPixelShader(const void * p_bytecode, size_t bytecode_length)
{
	HRESULT hr = mpDevice->CreatePixelShader(
		p_bytecode,
		bytecode_length,
		nullptr,
		&mpPixelShader
	);
	if(FAILED(hr))
	{
		return false;
	}

	return true;
}

bool GPUDeviceD3D11::createSwapChain()
{
	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc
	{
		.Width = mWidth,
		.Height = mHeight,
		.Format = mSwapChainBufferFormat,
		.Stereo = FALSE,
		.SampleDesc = { .Count = 1, .Quality = 0 },
		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		.BufferCount = mSwapChainBufferCount,
		.Scaling = DXGI_SCALING_STRETCH,
		.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
		.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
		.Flags = 0
	};

	ComPtr<IDXGISwapChain1> p_swap_chain;
	HRESULT hr = mpDXGIFactory->CreateSwapChainForHwnd(
		mpDevice.Get(),
		mhWnd,
		&swap_chain_desc,
		nullptr,
		nullptr,
		&p_swap_chain
	);
	if(FAILED(hr))
	{
		return false;
	}

	if(FAILED(p_swap_chain.As(&mpSwapChain)))
	{
		return false;
	}

	return true;
}

bool GPUDeviceD3D11::createRTV()
{
	ComPtr<ID3D11Texture2D> p_back_buffer;
	if(FAILED(mpSwapChain->GetBuffer(0, IID_PPV_ARGS(&p_back_buffer))))
	{
		return false;
	}

	HRESULT hr = mpDevice->CreateRenderTargetView(
		p_back_buffer.Get(),
		nullptr,
		&mpRTV
	);
	if(FAILED(hr))
	{
		return false;
	}

	return true;
}

bool GPUDeviceD3D11::setupGraphicsPipeline()
{
	// Input Assembler (IA)
	mpImmediateContext->IASetInputLayout(mpInputLayout.Get());
	mpImmediateContext->IASetVertexBuffers(
		0,
		1,
		mpVertexBuffer.GetAddressOf(),
		&mVertexStride,
		&mVertexOffset
	);
	mpImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Vertex Shader (VS)
	mpImmediateContext->VSSetShader(mpVertexShader.Get(), nullptr, 0);

	// Rasterizer Stage (RS)
	mpImmediateContext->RSSetViewports(1, &mViewport);

	// Pixel Shader (PS)
	mpImmediateContext->PSSetShader(mpPixelShader.Get(), nullptr, 0);

	// Output Merger (OM)
	mpImmediateContext->OMSetRenderTargets(1, mpRTV.GetAddressOf(), nullptr);

	return true;
}

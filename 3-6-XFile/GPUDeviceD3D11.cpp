#include "GPUDeviceD3D11.h"
#include <d3dcompiler.h>
#include <DirectXTex.h>
#include <numbers>
#include "xfile/XFileReader.h"

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

	bool createTextureSRV(ID3D11Device * p_device, const wchar_t * p_file_path, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> & p_srv)
	{
		DirectX::TexMetadata tex_meta_data;
		DirectX::ScratchImage scratch_image;

		HRESULT hr = DirectX::LoadFromWICFile(
			p_file_path,
			DirectX::WIC_FLAGS_NONE,
			&tex_meta_data,
			scratch_image
		);
		if(FAILED(hr))
		{
			return false;
		}

		hr = DirectX::CreateShaderResourceView(
			p_device,
			scratch_image.GetImage(0, 0, 0),
			1,
			tex_meta_data,
			&p_srv
		);
		if(FAILED(hr))
		{
			return false;
		}

		return true;
	}

#if 0
	struct Vertex
	{
		DirectX::XMFLOAT4 position;
		DirectX::XMFLOAT4 color;
	};

	std::vector<Vertex> vertices
	{
		{ DirectX::XMFLOAT4(-1.0f, -1.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ DirectX::XMFLOAT4( 1.0f, -1.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ DirectX::XMFLOAT4( 0.0f,  1.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
	};
#endif
}

GPUDeviceD3D11::~GPUDeviceD3D11()
{
	CoUninitialize();
}

bool GPUDeviceD3D11::initialize(HWND hWnd)
{
	if(FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
	{
		return false;
	}

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

	if(!loadMesh())
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

	if(!createIndexBuffer())
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

	if(!createVSConstantBuffer())
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

	if(!createRasterizerState())
	{
		return false;
	}

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

	if(!createSamplerState())
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

	if(!createBlendStates())
	{
		return false;
	}

	return true;
}

bool GPUDeviceD3D11::render()
{
	auto world = DirectX::XMMatrixIdentity();

	auto eye = DirectX::XMVectorSet(0.0f, 5.0f, -10.0f, 1.0f);
	auto at = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	auto up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
	auto view = DirectX::XMMatrixLookAtLH(eye, at, up);

	auto projection = DirectX::XMMatrixPerspectiveFovLH(
		std::numbers::pi_v<float> / 4.0f,
		static_cast<float>(mWidth) / static_cast<float>(mHeight),
		1.0f,
		100.0f
	);

	auto wvp = DirectX::XMMatrixTranspose(world * view * projection);

	D3D11_MAPPED_SUBRESOURCE mapped_subresource;
	HRESULT hr = mpImmediateContext->Map(
		mpVSConstantBuffer.Get(),
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&mapped_subresource
	);
	if(FAILED(hr))
	{
		return false;
	}

	memcpy(mapped_subresource.pData, &wvp, sizeof(wvp));

	mpImmediateContext->Unmap(mpVSConstantBuffer.Get(), 0);

	if(!setupGraphicsPipeline())
	{
		return false;
	}

	mpImmediateContext->ClearRenderTargetView(mpRTV.Get(), mClearColor);

	mpImmediateContext->DrawIndexed(countof(mIndices), 0, 0);

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

bool GPUDeviceD3D11::loadMesh()
{
	xfile::XFileReader reader;
	if(!reader.open("map.x"))
	{
		return false;
	}

	xfile::XFile xfile;
	if(!reader.read(xfile))
	{
		return false;
	}

	if(!reader.close())
	{
		return false;
	}

	if(xfile.meshes.size() != 1)
	{
		return false;
	}

	auto & vertices = xfile.meshes[0].vertices;
	auto & textureCoords = xfile.meshes[0].textureCoords.textureCoords;
	mVertices.resize(vertices.size());
	for(size_t i = 0; i < vertices.size(); ++i)
	{
		mVertices[i].position.x = vertices[i].x;
		mVertices[i].position.y = vertices[i].y;
		mVertices[i].position.z = vertices[i].z;
		mVertices[i].uv.x = textureCoords[i].u;
		mVertices[i].uv.y = textureCoords[i].v;
	}

	uint32_t index_count = 0;
	auto & faces = xfile.meshes[0].faces;
	for(const auto & face : faces)
	{
		index_count += static_cast<uint32_t>(face.faceVertexIndices.size());
	}

	mIndices.resize(index_count);
	for(size_t base = 0; const auto & face : faces)
	{
		size_t offset = 0;
		for(; offset < face.faceVertexIndices.size(); ++offset)
		{
			mIndices[base + offset] = face.faceVertexIndices[offset];
		}
		base += offset;
	}

	auto & material = xfile.meshes[0].materialList.materials[0];
	if(!material.textureFilename.filename.empty())
	{
		wchar_t path[MAX_PATH];
		mbstowcs(path, material.textureFilename.filename.c_str(), sizeof(path) / sizeof(path[0]));
		if(!createTextureSRV(mpDevice.Get(), path, mpTextureSRV))
		{
			return false;
		}
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
			.Format = DXGI_FORMAT_R32G32B32_FLOAT,
			.InputSlot = 0,
			.AlignedByteOffset = offsetof(Vertex, position),
			.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0
		},
		{
			.SemanticName = "TEXCOORD",
			.SemanticIndex = 0,
			.Format = DXGI_FORMAT_R32G32_FLOAT,
			.InputSlot = 0,
			.AlignedByteOffset = offsetof(Vertex, uv),
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
		.ByteWidth = bytesof(mVertices),
		.Usage = D3D11_USAGE_IMMUTABLE,
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
		.CPUAccessFlags = 0,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};

	D3D11_SUBRESOURCE_DATA subresource_data
	{
		.pSysMem = mVertices.data(),
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

	mVertexStride = sizeof(mVertices[0]);
	mVertexOffset = 0;

	return true;
}

bool GPUDeviceD3D11::createIndexBuffer()
{
	D3D11_BUFFER_DESC buffer_desc
	{
		.ByteWidth = bytesof(mIndices),
		.Usage = D3D11_USAGE_IMMUTABLE,
		.BindFlags = D3D11_BIND_INDEX_BUFFER,
		.CPUAccessFlags = 0,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};

	D3D11_SUBRESOURCE_DATA subresource_data
	{
		.pSysMem = mIndices.data(),
		.SysMemPitch = 0,
		.SysMemSlicePitch = 0
	};

	HRESULT hr = mpDevice->CreateBuffer(
		&buffer_desc,
		&subresource_data,
		&mpIndexBuffer
	);
	if(FAILED(hr))
	{
		return false;
	}

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

bool GPUDeviceD3D11::createVSConstantBuffer()
{
	D3D11_BUFFER_DESC buffer_desc
	{
		.ByteWidth = sizeof(DirectX::XMFLOAT4X4),
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};

	HRESULT hr = mpDevice->CreateBuffer(
		&buffer_desc,
		nullptr,
		&mpVSConstantBuffer
	);
	if(FAILED(hr))
	{
		return false;
	}

	return true;
}

bool GPUDeviceD3D11::createRasterizerState()
{
	D3D11_RASTERIZER_DESC rasterizer_desc
	{
		.FillMode = D3D11_FILL_SOLID,
		.CullMode = D3D11_CULL_NONE,
		.FrontCounterClockwise = FALSE,
		.DepthBias = D3D11_DEFAULT_DEPTH_BIAS,
		.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP,
		.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
		.DepthClipEnable = TRUE,
		.ScissorEnable = FALSE,
		.MultisampleEnable = FALSE,
		.AntialiasedLineEnable = FALSE
	};

	HRESULT hr = mpDevice->CreateRasterizerState(&rasterizer_desc, &mpRS);
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

bool GPUDeviceD3D11::createSamplerState()
{
	D3D11_SAMPLER_DESC sampler_desc
	{
		.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT,
		.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP,
		.MipLODBias = 0.0f,
		.MaxAnisotropy = 1,
		.ComparisonFunc = D3D11_COMPARISON_NEVER,
		.BorderColor = { 1.0f, 1.0f, 1.0f, 1.0f },
		.MinLOD = -FLT_MAX,
		.MaxLOD = FLT_MAX
	};

	HRESULT hr = mpDevice->CreateSamplerState(
		&sampler_desc,
		&mpSamplerState
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

bool GPUDeviceD3D11::createBlendStates()
{
	D3D11_BLEND_DESC blend_desc
	{
		.AlphaToCoverageEnable = FALSE,
		.IndependentBlendEnable = FALSE,
		.RenderTarget = {
			{
				.BlendEnable = FALSE,
				.SrcBlend = D3D11_BLEND_ONE,
				.DestBlend = D3D11_BLEND_ZERO,
				.BlendOp = D3D11_BLEND_OP_ADD,
				.SrcBlendAlpha = D3D11_BLEND_ZERO,
				.DestBlendAlpha = D3D11_BLEND_ONE,
				.BlendOpAlpha = D3D11_BLEND_OP_ADD,
				.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL
			},
		}
	};

	HRESULT hr = mpDevice->CreateBlendState(
		&blend_desc,
		&mpEarthBS
	);
	if(FAILED(hr))
	{
		return false;
	}

#define ALPHA_TYPE 0
	// Cd : Color destination
	// Cs : Color source
	// As : Alpha source

	blend_desc.RenderTarget[0] = {
		.BlendEnable = TRUE,
#if ALPHA_TYPE == 0
		// 線形合成 C = Cd(1 - As) + CsAs
		.SrcBlend = D3D11_BLEND_SRC_ALPHA,
		.DestBlend = D3D11_BLEND_INV_SRC_ALPHA,
		.BlendOp = D3D11_BLEND_OP_ADD,
#elif ALPHA_TYPE == 1
		// 加算合成 C = Cd + CsAs
		.SrcBlend = D3D11_BLEND_SRC_ALPHA,
		.DestBlend = D3D11_BLEND_ONE,
		.BlendOp = D3D11_BLEND_OP_ADD,
#elif ALPHA_TYPE == 2
		// 減算合成 C = Cd - CsAs
		.SrcBlend = D3D11_BLEND_SRC_ALPHA,
		.DestBlend = D3D11_BLEND_ONE,
		.BlendOp = D3D11_BLEND_OP_REV_SUBTRACT,
#elif ALPHA_TYPE == 3
		// 乗算合成 C = Cd * Cs
		.SrcBlend = D3D11_BLEND_ZERO,
		.DestBlend = D3D11_BLEND_SRC_COLOR,
		.BlendOp = D3D11_BLEND_OP_ADD,
#elif ALPHA_TYPE == 4
		// 焼き込み C = Cd * Cd
		.SrcBlend = D3D11_BLEND_ZERO,
		.DestBlend = D3D11_BLEND_DEST_COLOR,
		.BlendOp = D3D11_BLEND_OP_ADD,
#elif ALPHA_TYPE == 5
		// ネガポジ C = (1 - Cd) * Cs
		.SrcBlend = D3D11_BLEND_INV_DEST_COLOR,
		.DestBlend = D3D11_BLEND_ZERO,
		.BlendOp = D3D11_BLEND_OP_ADD,
#elif ALPHA_TYPE == 6
		// C = Cs
		.SrcBlend = D3D11_BLEND_ONE,
		.DestBlend = D3D11_BLEND_ZERO,
		.BlendOp = D3D11_BLEND_OP_ADD,
#endif
		.SrcBlendAlpha = D3D11_BLEND_ZERO,
		.DestBlendAlpha = D3D11_BLEND_ONE,
		.BlendOpAlpha = D3D11_BLEND_OP_ADD,
		.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL
	};

	hr = mpDevice->CreateBlendState(
		&blend_desc,
		&mpCloudBS
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
	mpImmediateContext->IASetIndexBuffer(mpIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	mpImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Vertex Shader (VS)
	mpImmediateContext->VSSetShader(mpVertexShader.Get(), nullptr, 0);
	mpImmediateContext->VSSetConstantBuffers(0, 1, mpVSConstantBuffer.GetAddressOf());

	// Rasterizer Stage (RS)
	mpImmediateContext->RSSetViewports(1, &mViewport);
	mpImmediateContext->RSSetState(mpRS.Get());

	// Pixel Shader (PS)
	mpImmediateContext->PSSetShader(mpPixelShader.Get(), nullptr, 0);
	mpImmediateContext->PSSetShaderResources(0, 1, mpTextureSRV.GetAddressOf());
	mpImmediateContext->PSSetSamplers(0, 1, mpSamplerState.GetAddressOf());

	// Output Merger (OM)
	mpImmediateContext->OMSetRenderTargets(1, mpRTV.GetAddressOf(), nullptr);

	return true;
}

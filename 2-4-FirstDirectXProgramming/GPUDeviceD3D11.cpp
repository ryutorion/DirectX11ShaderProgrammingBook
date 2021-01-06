#include "GPUDeviceD3D11.h"

namespace
{
	template <class T, size_t N>
	constexpr uint32_t countof(const T (&)[N]) noexcept
	{
		return static_cast<uint32_t>(N);
	}
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
	mpImmediateContext->OMSetRenderTargets(1, mpRTV.GetAddressOf(), nullptr);

	return true;
}

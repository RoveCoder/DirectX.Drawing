#include "Renderer.h"
#include <SDL_syswm.h>
#include <d3d11_1.h>
#include <DirectXColors.h>
#include <fstream>
#include <SDL_messagebox.h>

Renderer::Renderer(SDL_Window* sdlWindow) : m_SdlWindow(sdlWindow)
{
}

bool Renderer::Init()
{
	int width = 0;
	int height = 0;
	SDL_GetWindowSize(m_SdlWindow, &width, &height);

	CreateDevice();
	CreateSwapChain(width, height);

	CreateRenderTargetAndDepthStencilView(width, height);
	SetViewport(width, height);

	CreateRasterStateSolid();
	CreateRasterStateWireframe();

	return true;
}

void Renderer::Quit()
{
}

void Renderer::Resize(int width, int height)
{
	m_RenderTargetView->Release();
	m_DepthStencilView->Release();

	DX::ThrowIfFailed(m_SwapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	CreateRenderTargetAndDepthStencilView(width, height);
	SetViewport(width, height);
}

void Renderer::Clear()
{
	m_DeviceContext->ClearRenderTargetView(m_RenderTargetView, reinterpret_cast<const float*>(&DirectX::Colors::SteelBlue));
	m_DeviceContext->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void Renderer::Render()
{
	m_SwapChain->Present(0, 0);
}

void Renderer::EnableWireframe(bool enable)
{
	if (enable)
	{
		m_DeviceContext->RSSetState(m_RasterStateWireframe);
	}
	else
	{
		m_DeviceContext->RSSetState(m_RasterStateSolid);
	}
}

void Renderer::CreateDevice()
{
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	D3D_FEATURE_LEVEL featureLevel;
	DX::ThrowIfFailed(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &m_Device, &featureLevel, &m_DeviceContext));

	// Check for x4 multisampling
	DX::ThrowIfFailed(m_Device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_4xMsaaQuality));
}

void Renderer::CreateSwapChain(int width, int height)
{
	// Get DXIFactory
	IDXGIDevice* dxgiDevice = nullptr;
	DX::ThrowIfFailed(m_Device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice)));

	IDXGIAdapter* adapter = nullptr;
	DX::ThrowIfFailed(dxgiDevice->GetAdapter(&adapter));

	IDXGIFactory1* dxgiFactory = nullptr;
	DX::ThrowIfFailed(adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory)));

	adapter->Release();
	dxgiDevice->Release();

	IDXGIFactory2* dxgiFactory2 = nullptr;
	DX::ThrowIfFailed(dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2)));

	if (dxgiFactory2 != nullptr)
	{
		// DirectX 11.1
		DXGI_SWAP_CHAIN_DESC1 sd = {};
		sd.Width = width;
		sd.Height = height;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.SampleDesc.Count = 4;
		sd.SampleDesc.Quality = m_4xMsaaQuality - 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;

		IDXGISwapChain1* swapChain1 = nullptr;
		DX::ThrowIfFailed(dxgiFactory2->CreateSwapChainForHwnd(m_Device, GetHWND(), &sd, nullptr, nullptr, &swapChain1));
		DX::ThrowIfFailed(swapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&m_SwapChain)));

		dxgiFactory2->Release();
	}
	else
	{
		// DirectX 11
		DXGI_SWAP_CHAIN_DESC sd = {};
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = GetHWND();
		sd.SampleDesc.Count = 4;
		sd.SampleDesc.Quality = m_4xMsaaQuality - 1;
		sd.Windowed = TRUE;

		DX::ThrowIfFailed(dxgiFactory->CreateSwapChain(m_Device, &sd, &m_SwapChain));
	}

	dxgiFactory2->Release();
}

void Renderer::CreateRenderTargetAndDepthStencilView(int width, int height)
{
	// Render target view
	ID3D11Resource* backBuffer = nullptr;
	m_SwapChain->GetBuffer(0, __uuidof(ID3D11Resource), reinterpret_cast<void**>(&backBuffer));
	DX::ThrowIfFailed(m_Device->CreateRenderTargetView(backBuffer, nullptr, &m_RenderTargetView));
	backBuffer->Release();

	// Depth stencil view
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 4;
	descDepth.SampleDesc.Quality = m_4xMsaaQuality - 1;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	ID3D11Texture2D* depthStencil;
	DX::ThrowIfFailed(m_Device->CreateTexture2D(&descDepth, nullptr, &depthStencil));
	DX::ThrowIfFailed(m_Device->CreateDepthStencilView(depthStencil, nullptr, &m_DepthStencilView));

	m_DeviceContext->OMSetRenderTargets(1, &m_RenderTargetView, m_DepthStencilView);
}

void Renderer::SetViewport(int width, int height)
{
	D3D11_VIEWPORT viewport;
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	m_DeviceContext->RSSetViewports(1, &viewport);
}

HWND Renderer::GetHWND()
{
	SDL_SysWMinfo wmInfo;
	SDL_GetVersion(&wmInfo.version);
	SDL_GetWindowWMInfo(m_SdlWindow, &wmInfo);
	return wmInfo.info.win.window;
}

void Renderer::CreateRasterStateSolid()
{
	D3D11_RASTERIZER_DESC rasterizerState;
	ZeroMemory(&rasterizerState, sizeof(D3D11_RASTERIZER_DESC));

	rasterizerState.AntialiasedLineEnable = true;
	rasterizerState.CullMode = D3D11_CULL_FRONT;
	rasterizerState.FillMode = D3D11_FILL_SOLID;
	rasterizerState.DepthClipEnable = true;
	rasterizerState.FrontCounterClockwise = true;
	rasterizerState.MultisampleEnable = true;

	DX::ThrowIfFailed(m_Device->CreateRasterizerState(&rasterizerState, &m_RasterStateSolid));
}

void Renderer::CreateRasterStateWireframe()
{
	D3D11_RASTERIZER_DESC rasterizerState;
	ZeroMemory(&rasterizerState, sizeof(D3D11_RASTERIZER_DESC));

	rasterizerState.AntialiasedLineEnable = true;
	rasterizerState.CullMode = D3D11_CULL_NONE;
	rasterizerState.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerState.DepthClipEnable = true;
	rasterizerState.FrontCounterClockwise = true;
	rasterizerState.MultisampleEnable = true;

	DX::ThrowIfFailed(m_Device->CreateRasterizerState(&rasterizerState, &m_RasterStateWireframe));
}

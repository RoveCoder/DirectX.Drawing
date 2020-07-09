#pragma once

#include <d3d11.h>
#include <SDL_video.h>
#include <exception>
#include <string>

class Renderer
{
public:
	Renderer(SDL_Window* sdlWindow);

	bool Init();
	void Quit();

	void Resize(int width, int height);

	void Clear();
	void Render();

	constexpr ID3D11Device* GetDevice() { return m_Device; }
	constexpr ID3D11DeviceContext* GetDeviceContext() { return m_DeviceContext; }

	void EnableWireframe(bool enable);

private:
	SDL_Window* m_SdlWindow = nullptr;

	UINT m_4xMsaaQuality = 0;

	ID3D11Device* m_Device = nullptr;
	ID3D11DeviceContext* m_DeviceContext = nullptr;
	IDXGISwapChain* m_SwapChain = nullptr;

	ID3D11RenderTargetView* m_RenderTargetView = nullptr;
	ID3D11DepthStencilView* m_DepthStencilView = nullptr;

	void CreateDevice();
	void CreateSwapChain(int width, int height);

	void CreateRenderTargetAndDepthStencilView(int width, int height);
	void SetViewport(int width, int height);

	HWND GetHWND();

	// Raster states
	void CreateRasterStateSolid();
	void CreateRasterStateWireframe();

	ID3D11RasterizerState* m_RasterStateSolid = nullptr;
	ID3D11RasterizerState* m_RasterStateWireframe = nullptr;
};

namespace DX
{
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			throw std::exception();
		}
	}
}
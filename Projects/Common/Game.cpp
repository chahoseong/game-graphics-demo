module;
#include <cassert>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl.h>

export module core;

import <filesystem>;
import <format>;
import <iostream>;
import <map>;
import <memory>;
import <span>;
import <string>;
import <vector>;

import utility;

export class Game
{
public:
	Game(std::wstring_view title, int width, int height, bool windowed);
	~Game();

	virtual bool Startup(HWND window);
	virtual void Shutdown();

	void Update();
	void Render();

	void Resize(int width, int height);
	void Resume();
	void Pause();

	std::wstring GetAssetPath(std::wstring_view filename) const;

	std::wstring_view Title() const { return title_; }
	int ScreenWidth() const { return screenWidth_; }
	int ScreenHeight() const { return screenHeight_; }
	bool IsPaused() const { return paused_; }

protected:
	virtual void Update(float deltaTime) { }
	virtual void Render(ID3D11DeviceContext* immediateContext) { }

	ID3D11Device* GraphicsDevice() const& { return graphicsDevice_.Get(); }
	// ID3D11DeviceContext* ImmediateContext() const& { return immediateContext_.Get(); }
	// IDXGISwapChain* SwapChain() const& { return swapChain_.Get(); }
	// ID3D11RenderTargetView* RenderTargetView() const& { return renderTargetView_.Get(); }
	// ID3D11DepthStencilView* DepthStencilView() const& { return depthStencilView_.Get(); }

	void SetBackgroundColor(float r, float g, float b, float a);

private:
	bool InitDirect3D(HWND window);
	DXGI_RATIONAL FindRefreshRate(IDXGIAdapter* adapter) const;

private:
	std::wstring title_;
	int screenWidth_ = 0;
	int screenHeight_ = 0;

	mutable std::wstring assetDirectory_;

	bool windowed_ = true;
	bool paused_ = false;

	// graphics 
	Microsoft::WRL::ComPtr<ID3D11Device> graphicsDevice_;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext_;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain_;
	
	std::map<void*, Microsoft::WRL::ComPtr<ID3D11RenderTargetView>> renderTargetViewCache_;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> currentRenderTargetView_;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView_;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer_;
	
	D3D_DRIVER_TYPE driverType_ = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT backBufferFormat_ = DXGI_FORMAT_R8G8B8A8_UNORM;
	D3D11_VIEWPORT viewport_;
	DirectX::XMFLOAT4 backgroundColor_ = { 0.69f, 0.77f, 0.87f, 1.0f };
};

module :private;

Game::Game(std::wstring_view title, int width, int height, bool windowed)
	: title_(title), screenWidth_(width), screenHeight_(height), windowed_(windowed)
{

}

Game::~Game()
{

}

bool Game::Startup(HWND window)
{
	if (!InitDirect3D(window)) {
		return false;
	}

	return true;
}

bool Game::InitDirect3D(HWND window)
{
	// Create the device and context

	UINT flags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr = D3D11CreateDevice(
		nullptr,
		driverType_,
		NULL,
		flags,
		nullptr, 0,
		D3D11_SDK_VERSION,
		graphicsDevice_.GetAddressOf(),
		&featureLevel,
		immediateContext_.GetAddressOf()
	);

	if (FAILED(hr)) {
		std::cerr << "Failed to create Direct3D device\n";
		return false;
	}

	if (featureLevel != D3D_FEATURE_LEVEL_11_0) {
		std::cerr << "Direct3D feature level 11 unsupported\n";
		return false;
	}

	Microsoft::WRL::ComPtr<IDXGIDevice> device = nullptr;
	ThrowIfFailed(graphicsDevice_->QueryInterface(IID_PPV_ARGS(&device)));

	Microsoft::WRL::ComPtr<IDXGIAdapter> adapter = nullptr;
	ThrowIfFailed(device->GetParent(IID_PPV_ARGS(&adapter)));

	DXGI_RATIONAL refreshRate = FindRefreshRate(adapter.Get());

	// Fill out a DXGI_SWAP_CHAIN_DESC to describe swap chain

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.BufferDesc.Width = screenWidth_;
	swapChainDesc.BufferDesc.Height = screenHeight_;
	swapChainDesc.BufferDesc.RefreshRate = refreshRate;
	swapChainDesc.BufferDesc.Format = backBufferFormat_;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.OutputWindow = window;
	swapChainDesc.Windowed = windowed_;
	swapChainDesc.Flags = 0;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	// To correctly create the swap chain, we must use the IDXGIFactory that was
	// used to create the device.

	Microsoft::WRL::ComPtr<IDXGIFactory> factory;
	ThrowIfFailed(adapter->GetParent(IID_PPV_ARGS(&factory)));

	hr = factory->CreateSwapChain(graphicsDevice_.Get(), &swapChainDesc, swapChain_.GetAddressOf());
	if (FAILED(hr))
	{
		std::cerr << "Failed to create swap chain\n";
		return false;
	}

	// The remaining steps that need to be carried out for
	// direct3d creation also need to be executed every time
	// the window is resized. So just call the Resize() method
	// here to avoid code duplication

	Resize(screenWidth_, screenHeight_);

	return true;
}

DXGI_RATIONAL Game::FindRefreshRate(IDXGIAdapter* adapter) const
{
	DXGI_RATIONAL refreshRate{ .Numerator = 60, .Denominator = 1 };

	// 창모드에서는 numerator = 0, denominator = 1을 설정하면
	// 가장 적합한 Refresh Rate를 설정합니다.
	if (windowed_) {
		refreshRate.Numerator = 0;
		refreshRate.Denominator = 1;
	}
	else {
		Microsoft::WRL::ComPtr<IDXGIOutput> output;
		adapter->EnumOutputs(0, &output);

		UINT numModes = 0;
		ThrowIfFailed(output->GetDisplayModeList(backBufferFormat_, 0, &numModes, nullptr));

		std::vector<DXGI_MODE_DESC> modeList(numModes);
		ThrowIfFailed(output->GetDisplayModeList(backBufferFormat_, 0, &numModes, &modeList[0]));

		for (UINT i = 0; i < numModes; ++i) {
			if (modeList[i].Width == screenWidth_ && modeList[i].Height == screenHeight_) {
				refreshRate = modeList[i].RefreshRate;
				break;
			}
		}
	}

	return refreshRate;
}

void Game::Shutdown()
{
	if (immediateContext_) {
		immediateContext_->ClearState();
	}
}

void Game::Update()
{
	Update(0.0f);
}

void Game::Render()
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
	ThrowIfFailed(swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));

	if (auto it = renderTargetViewCache_.find(backBuffer.Get()); it != renderTargetViewCache_.end()) {
		currentRenderTargetView_ = it->second;
	}
	else {
		ThrowIfFailed(graphicsDevice_->CreateRenderTargetView(backBuffer.Get(), nullptr, currentRenderTargetView_.GetAddressOf()));
		renderTargetViewCache_.insert({ backBuffer.Get(), currentRenderTargetView_ });
	}

	immediateContext_->OMSetRenderTargets(1, currentRenderTargetView_.GetAddressOf(), depthStencilView_.Get());

	immediateContext_->ClearRenderTargetView(currentRenderTargetView_.Get(), reinterpret_cast<const float*>(&backgroundColor_));
	immediateContext_->ClearDepthStencilView(depthStencilView_.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	Render(immediateContext_.Get());

	ThrowIfFailed(swapChain_->Present(0, 0));
}

void Game::Resize(int width, int height)
{
	assert(graphicsDevice_);
	assert(immediateContext_);
	assert(swapChain_);

	immediateContext_->OMSetRenderTargets(0, nullptr, nullptr);
	currentRenderTargetView_.Reset();
	renderTargetViewCache_.clear();

	depthStencilView_.Reset();
	depthStencilBuffer_.Reset();

	// Resize the swap chain
	ThrowIfFailed(swapChain_->ResizeBuffers(0, width, height, backBufferFormat_, 0));

	// Create the depth/stencil buffer and view
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	ThrowIfFailed(graphicsDevice_->CreateTexture2D(&depthStencilDesc, nullptr, depthStencilBuffer_.GetAddressOf()));
	ThrowIfFailed(graphicsDevice_->CreateDepthStencilView(depthStencilBuffer_.Get(), nullptr, depthStencilView_.GetAddressOf()));

	// Set the viewport transform
	viewport_.TopLeftX = 0;
	viewport_.TopLeftY = 0;
	viewport_.Width = static_cast<float>(width);
	viewport_.Height = static_cast<float>(height);
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;
	immediateContext_->RSSetViewports(1, &viewport_);
}

void Game::Resume()
{
	paused_ = false;
}

void Game::Pause()
{
	paused_ = true;
}

std::wstring Game::GetAssetPath(std::wstring_view filename) const
{
	if (assetDirectory_.empty()) {
		std::filesystem::path path = std::filesystem::current_path();
		while (path.has_parent_path()) {
			for (const auto& entry : std::filesystem::directory_iterator(path)) {
				if (entry.is_directory() && entry.path().filename() == L"Assets") {
					assetDirectory_ = entry.path();
					break;
				}
			}
			if (assetDirectory_.empty()) {
				path = path.parent_path();
			}
			else {
				break;
			}
		}
	}
	std::wstring assetPath = std::format(L"{}{}{}", assetDirectory_, std::filesystem::path::preferred_separator, filename);
	return assetPath;
}

void Game::SetBackgroundColor(float r, float g, float b, float a)
{
	backgroundColor_.x = r;
	backgroundColor_.y = g;
	backgroundColor_.z = b;
	backgroundColor_.w = a;
}

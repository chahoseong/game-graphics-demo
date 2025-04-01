module;
// C
#include <cstdlib>

// Windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// ImGui
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam);

export module platform.windows;

import <iostream>;
import <memory>;
import <string>;

import core;

export class Application
{
public:
	static int Run(Game* game);
	static HWND NativeHandle();

private:
	static bool InitInstance(Game* game);
	static bool InitImGui(Game* game);

	static LRESULT CALLBACK HandleMessage(HWND, UINT, WPARAM, LPARAM);

private:
	static std::unique_ptr<Application> instance_;

public:
	~Application();

private:
	explicit Application(HWND hWnd);

private:
	HWND window_;
	bool minimized_;
	bool maximized_;
	bool resizing_;
};

module :private;

std::unique_ptr<Application> Application::instance_;

Application::Application(HWND hWnd)
	: window_(hWnd)
{
}

Application::~Application()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

int Application::Run(Game* game)
{
	if (!InitInstance(game)) {
		return EXIT_FAILURE;
	}

	if (!game->Startup(instance_->window_)) {
		return EXIT_FAILURE;
	}

	if (!InitImGui(game)) {
		return EXIT_FAILURE;
	}

	ShowWindow(instance_->window_, SW_SHOWDEFAULT);
	UpdateWindow(instance_->window_);

	MSG msg = { 0 };
	
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			if (!game->IsPaused()) {
				// Start the Dear ImGui frame
				ImGui_ImplDX11_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame();

				game->Update();
				game->Render();
				
				// Render the Dear ImGui frame
				ImGui::Render();
				ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

				game->Present();
			}
			else {
				Sleep(100);
			}
		}
	}

	game->Shutdown();

	return static_cast<int>(msg.wParam);
}

bool Application::InitInstance(Game* game)
{
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = HandleMessage;
	windowClass.hInstance = GetModuleHandle(NULL);
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = L"GameGraphicsApp";

	if (!RegisterClassEx(&windowClass)) {
		std::cerr << "Failed to register window class";
		return false;
	}

	RECT rect = { 0, 0, static_cast<LONG>(game->ScreenWidth()), static_cast<LONG>(game->ScreenHeight()) };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

	HWND hWnd = CreateWindow(
		windowClass.lpszClassName,
		game->Title().data(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left,
		rect.bottom - rect.top,
		NULL,
		NULL,
		windowClass.hInstance,
		game
	);
	if (!hWnd) {
		std::cerr << "Failed to create window";
		return false;
	}

	instance_ = std::unique_ptr<Application>(new Application(hWnd));

	return true;
}

bool Application::InitImGui(Game* game)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.DisplaySize = ImVec2(static_cast<float>(game->ScreenWidth()), static_cast<float>(game->ScreenHeight()));

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	// ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	if (!ImGui_ImplWin32_Init(instance_->window_)) {
		std::cerr << "Failed to init ImGui win32\n";
		return false;
	}

	if (!ImGui_ImplDX11_Init(game->GraphicsDevice(), game->ImmediateContext())) {
		std::cerr << "Failed to init ImGui dx11\n";
		return false;
	}

	return true;
}

LRESULT CALLBACK Application::HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam)) {
		return true;
	}

	Game* game = reinterpret_cast<Game*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (message) {
	case WM_CREATE:
	{
		LPCREATESTRUCT createStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(createStruct->lpCreateParams));
	}
	return 0;
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE) {
			game->Pause();
		}
		else {
			game->Resume();
		}
		return 0;
	case WM_SIZE:
		if (game) {
			int width = LOWORD(lParam);
			int height = HIWORD(lParam);
			if (wParam == SIZE_MINIMIZED) {
				instance_->minimized_ = true;
				instance_->maximized_ = false;
				game->Pause();
			}
			else if (wParam == SIZE_MAXIMIZED) {
				instance_->minimized_ = false;
				instance_->maximized_ = true;
				game->Resize(width, height);
				game->Resume();
			}
			else if (wParam == SIZE_RESTORED) {
				if (instance_->minimized_) {
					instance_->minimized_ = false;
					game->Resize(width, height);
					game->Resume();
				}
				else if (instance_->maximized_) {
					instance_->maximized_ = false;
					game->Resize(width, height);
					game->Resume();
				}
				else if (instance_->resizing_) {

				}
				else {
					game->Resize(width, height);
				}
			}
		}
		return 0;
	case WM_ENTERSIZEMOVE:
		instance_->resizing_ = true;
		if (game) {
			game->Pause();
		}
		return 0;
	case WM_EXITSIZEMOVE:
		instance_->resizing_ = false;
		if (game) {
			RECT screenRect;
			GetClientRect(hWnd, &screenRect);
			game->Resize(screenRect.right - screenRect.left, screenRect.bottom - screenRect.top);
			game->Resume();
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

HWND Application::NativeHandle()
{
	return instance_ ? instance_->window_ : NULL;
}

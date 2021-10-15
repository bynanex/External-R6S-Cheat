
#include "globals.hpp"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_internal.h"
#include "imgui/Comfortaa-Regular.h"
#include "imgui/FontAwesome.h"
#include "imgui/IconsFontAwesome.h"
#include "imgui/imgui_internal.h"
#include <DXGI.h>
#include <D3D11.h>
#include <string>
#include <dwmapi.h>
#include <thread>
#include "Overlay.hpp"

#pragma comment (lib, "d3d11.lib")

HWND overlayWindow = NULL;

ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
IDXGISwapChain* pSwapChain = NULL;
ID3D11RenderTargetView* pMainRenderTargetView = NULL;
ImDrawList* drawList = NULL;
ImFont* comfortaa = NULL;
ImFont* bigFont = NULL;
std::string menuTitle = ("Cedy.cc R6 Private");
std::string topText = ("Cedy.cc | Private Build | Rainbow Six Siege");
const int configButtonSize = 40;

bool done = false;
bool result = false;

void Style()
{
	ImGuiStyle& Style = ImGui::GetStyle();
	auto Color = Style.Colors;

	Style.WindowBorderSize = 0.f;

	Style.ChildRounding = 0.f;
	Style.FrameRounding = 0.f;
	Style.ScrollbarRounding = 0.f;
	Style.GrabRounding = 0.f;
	Style.PopupRounding = 0.f;
	Style.WindowRounding = 3.f;


	Color[ImGuiCol_WindowBg] = ImColor(18, 18, 18, 255);

	Color[ImGuiCol_FrameBg] = ImColor(31, 31, 31, 255);
	Color[ImGuiCol_FrameBgActive] = ImColor(41, 41, 41, 255);
	Color[ImGuiCol_FrameBgHovered] = ImColor(41, 41, 41, 255);

	Color[ImGuiCol_Button] = ImColor(29, 29, 29, 255);
	Color[ImGuiCol_ButtonActive] = ImColor(32, 32, 32, 255);
	Color[ImGuiCol_ButtonHovered] = ImColor(36, 36, 36, 255);

	Color[ImGuiCol_Border] = ImColor(0, 0, 0, 0);
	Color[ImGuiCol_Separator] = ImColor(36, 36, 36, 255);

	Color[ImGuiCol_ResizeGrip] = ImColor(30, 30, 30, 255);
	Color[ImGuiCol_ResizeGripActive] = ImColor(30, 30, 30, 255);
	Color[ImGuiCol_ResizeGripHovered] = ImColor(30, 30, 30, 255);

	Color[ImGuiCol_ChildBg] = ImColor(0, 0, 0, 0);

	Color[ImGuiCol_ScrollbarBg] = ImColor(24, 24, 24, 255);
	Color[ImGuiCol_ScrollbarGrab] = ImColor(24, 24, 24, 255);
	Color[ImGuiCol_ScrollbarGrabActive] = ImColor(24, 24, 24, 255);
	Color[ImGuiCol_ScrollbarGrabActive] = ImColor(24, 24, 24, 255);

	Color[ImGuiCol_Header] = ImColor(39, 39, 39, 255);
	Color[ImGuiCol_HeaderActive] = ImColor(39, 39, 39, 255);
	Color[ImGuiCol_HeaderHovered] = ImColor(39, 39, 39, 255);
	Color[ImGuiCol_CheckMark] = ImColor(255, 255, 255, 255);
}

void Line(int newId)
{
	std::string id = ("imguipp_line_" + std::to_string(newId));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 0, 0, 0));
	{
		ImGui::BeginChild(id.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 1), false);
		ImGui::Separator();
		ImGui::EndChild();
	}
	ImGui::PopStyleColor();
}

void Linevertical()
{
	ImGui::SameLine();
	ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
	ImGui::SameLine();
}

void CenterText(const char* text, int lineId, bool separator)
{
	if (text == nullptr)
		return;

	ImGui::Spacing();
	ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (ImGui::CalcTextSize(text).x / 2));
	ImGui::Text(text);
	ImGui::Spacing();

	if (separator)
		Line(lineId);
}

void CenterTextEx(const char* text, float width, int lineId, bool separator)
{
	if (text == nullptr)
		return;

	ImGui::Spacing();
	ImGui::SameLine((width / 2) - (ImGui::CalcTextSize(text).x / 2));
	ImGui::Text(text);
	ImGui::Spacing();

	if (separator)
		Line(lineId);
}

void CleanupRenderTarget()
{
	if (pMainRenderTargetView) { pMainRenderTargetView->Release(); pMainRenderTargetView = NULL; }
}

void CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	if (pBackBuffer)
	{
		pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pMainRenderTargetView);
		pBackBuffer->Release();
	}
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (pDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			CleanupRenderTarget();
			pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			CreateRenderTarget();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

bool InitDX11()
{
	DXGI_SWAP_CHAIN_DESC params;
	ZeroMemory(&params, sizeof(params));

	params.BufferCount = 2;
	params.BufferDesc.Width = (UINT)globals.width;
	params.BufferDesc.Height = (UINT)globals.height;
	params.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	params.BufferDesc.RefreshRate.Numerator = 144;
	params.BufferDesc.RefreshRate.Denominator = 1;
	params.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	params.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	params.OutputWindow = overlayWindow;
	params.SampleDesc.Count = 1;
	params.SampleDesc.Quality = 0;
	params.Windowed = TRUE;
	params.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;

	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

	if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &params, &pSwapChain, &pDevice, &featureLevel, &pContext) != S_OK)
		return false;

	CreateRenderTarget();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	io.IniFilename = NULL;

	comfortaa = io.Fonts->AddFontFromMemoryTTF(comfortaaRegular, sizeof(comfortaaRegular), 13.f);
	bigFont = io.Fonts->AddFontFromMemoryTTF(comfortaaRegular, sizeof(comfortaaRegular), 20.f);

	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.GlyphOffset = ImVec2(0.f, 2.f);

	io.Fonts->AddFontFromMemoryTTF(font_awesome_data, sizeof(font_awesome_data), 20.f, &icons_config, icons_ranges);

	ImGui_ImplWin32_Init(overlayWindow);
	ImGui_ImplDX11_Init(pDevice, pContext);

	Style();

	return true;
}


const float ClearColor[4] = { 0.f, 0.f, 0.f, 0.f };

void RenderFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y), ImGuiCond_Always);
	ImGui::Begin(("##scene"), (bool*)0, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar);

	drawList = ImGui::GetWindowDrawList();

	drawList->AddText(comfortaa, 13.f, ImVec2(10, 10), IM_COL32(255, 255, 255, 255), topText.c_str());


	drawList->PushClipRectFullScreen();
	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);

	ImGui::EndFrame();

	ImGui::Render();

	pContext->OMSetRenderTargets(1, &pMainRenderTargetView, NULL);
	pContext->ClearRenderTargetView(pMainRenderTargetView, ClearColor);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	pSwapChain->Present(1, 0);
}

void Update()
{
	HWND activeWnd = GetForegroundWindow();
	if (activeWnd == globals.hWnd)
	{
		HWND hwndtest = GetWindow(activeWnd, GW_HWNDPREV);
		SetWindowPos(overlayWindow, hwndtest, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}

	//SetWindowLongA(overlayWindow, GWL_EXSTYLE, (WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT));
	SetWindowLongA(overlayWindow, GWL_EXSTYLE, (WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT));
}

WPARAM WindowLoop(WNDCLASSEXA wClass)
{
	int xOff = rand() % 15;
	int yOff = rand() % 15;
	overlayWindow = CreateWindowExA(0, wClass.lpszClassName, wClass.lpszMenuName, WS_POPUP | WS_VISIBLE, 0, 0, (int)(globals.width + xOff), (int)(globals.height + yOff), 0, 0, 0, 0);

	if (!overlayWindow)
		exit(0);

	MARGINS margin = { -4, -2, (int)(globals.width + xOff), (int)(globals.height + yOff) };
	DwmExtendFrameIntoClientArea(overlayWindow, &margin);

	SetWindowLongA(overlayWindow, GWL_EXSTYLE, (WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT));
	ShowWindow(overlayWindow, SW_SHOW);
	UpdateWindow(overlayWindow);

	result = InitDX11();
	done = true;
	if (!result)
		return 0;

	MSG message = { 0 };
	while (message.message != WM_QUIT)
	{
		if (PeekMessageA(&message, overlayWindow, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessageA(&message);
		}

		Update();
		RenderFrame();
	}

	overlay::DestroyWindow();
}

bool overlay::InitWindow()
{
	std::string windowName = "lolz";
	std::string windowClassName = "lolz";

	WNDCLASSEXA wClass;
	wClass.cbSize = sizeof(WNDCLASSEXA);
	wClass.style = 0;
	wClass.lpfnWndProc = WndProc;
	wClass.cbClsExtra = 0;
	wClass.cbWndExtra = 0;
	wClass.hInstance = nullptr;
	wClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wClass.hbrBackground = nullptr;
	wClass.lpszMenuName = windowName.c_str();
	wClass.lpszClassName = windowClassName.c_str();
	wClass.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

	if (!RegisterClassExA(&wClass))
		return false;

	std::thread(WindowLoop, wClass).detach();

	while (!done) { Sleep(1); }

	return result;
}

void overlay::DestroyWindow()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	CleanupRenderTarget();
	if (pSwapChain) { pSwapChain->Release(); pSwapChain = NULL; }
	if (pContext) { pContext->Release(); pContext = NULL; }
	if (pDevice) { pDevice->Release(); pDevice = NULL; }
	DestroyWindow(overlayWindow);
}
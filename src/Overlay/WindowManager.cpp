#include "WindowManager.h"

#include "fonts.h"
#include "WindowContainer/WindowContainer.h"
#include "Window/LogWindow.h"

#include "Core/interfaces.h"
#include "Core/logger.h"
#include "Core/Settings.h"
#include "Core/utils.h"
#include "Web/update_check.h"
#include <imgui.h>
#include <imgui_impl_dx9.h>

#include <ctime>
#include <sstream>

#define DEFAULT_ALPHA 0.87f

int keyToggleMainWindow;
int keyToggleHud;

WindowManager* WindowManager::m_instance = nullptr;

WindowManager & WindowManager::GetInstance()
{
	if (m_instance == nullptr)
	{
		m_instance = new WindowManager();
	}
	return *m_instance;
}

bool WindowManager::Initialize(void *hwnd, IDirect3DDevice9 *device)
{
	if (m_initialized)
	{
		return true;
	}

	LOG(2, "WindowManager::Initialize\n");

	if (!hwnd)
	{
		LOG(2, "HWND not found!\n");
		return false;
	}
	if (!device)
	{
		LOG(2, "Direct3DDevice9 not found!\n");
		return false;
	}

	m_initialized = ImGui_ImplDX9_Init(hwnd, device);
	if (!m_initialized)
	{
		LOG(2, "ImGui_ImplDX9_Init failed!\n");
		return false;
	}

	m_pLogger = g_imGuiLogger;

	m_pLogger->Log("[system] Initialization starting...\n");

	m_windowContainer = new WindowContainer();

	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowBorderSize = 1;
	style.FrameBorderSize = 1;
	style.ScrollbarSize = 14;
	style.Alpha = DEFAULT_ALPHA;

	if (Settings::settingsIni.menusize == 1)
	{
		ImFont* font = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(TinyFont_compressed_data_base85, 10);
		font->DisplayOffset.y += 1;
	}
	else if (Settings::settingsIni.menusize == 3)
	{
		ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(DroidSans_compressed_data, DroidSans_compressed_size, 20);
	}

	keyToggleMainWindow = Settings::getButtonValue(Settings::settingsIni.togglebutton);
	m_pLogger->Log("[system] Toggling key set to '%s'\n", Settings::settingsIni.togglebutton.c_str());

	keyToggleHud = Settings::getButtonValue(Settings::settingsIni.toggleHUDbutton);
	m_pLogger->Log("[system] HUD toggling key set to '%s'\n", Settings::settingsIni.toggleHUDbutton.c_str());

	g_interfaces.pPaletteManager->LoadAllPalettes();

	/*Calling a frame to initialize beforehand to prevent a crash upon first call of
	/Update if the game window is not focused. Simply calling ImGui_ImplDX9_CreateDeviceObjects() might be enough too.*/
	ImGui_ImplDX9_NewFrame();
	ImGui::EndFrame();
	///////

	srand(time(NULL));

	//StartAsyncUpdateCheck();

	m_pLogger->Log("[system] Finished initialization\n");
	m_pLogger->LogSeparator();
	LOG(2, "Initialize end\n");

	return true;
}

void WindowManager::Shutdown()
{
	if (!m_initialized)
	{
		return;
	}

	LOG(2, "WindowManager::Shutdown\n");

	SAFE_DELETE(m_windowContainer);
	delete m_instance;

	ImGui_ImplDX9_Shutdown();
}

void WindowManager::InvalidateDeviceObjects()
{
	if (!m_initialized)
	{
		return;
	}

	LOG(2, "WindowManager::InvalidateDeviceObjects\n");
	ImGui_ImplDX9_InvalidateDeviceObjects();
}

void WindowManager::CreateDeviceObjects()
{
	if (!m_initialized)
	{
		return;
	}

	LOG(2, "WindowManager::CreateDeviceObjects\n");
	ImGui_ImplDX9_CreateDeviceObjects();
}

void WindowManager::Render()
{
	if (!m_initialized)
	{
		return;
	}

	LOG(7, "WindowManager::Render\n");

	HandleButtons();

	ImGui_ImplDX9_NewFrame();

	bool isMainWindowOpen =
		m_windowContainer->GetWindow(WindowType_Main)->IsOpen();
	bool isUpdateNotifierWindowOpen =
		m_windowContainer->GetWindow(WindowType_UpdateNotifier)->IsOpen();
	bool isPaletteEditorWindowOpen =
		m_windowContainer->GetWindow(WindowType_PaletteEditor)->IsOpen();
	bool isLogWindowOpen =
		m_windowContainer->GetWindow(WindowType_Log)->IsOpen();
	bool isDebugWindowOpen =
		m_windowContainer->GetWindow(WindowType_Debug)->IsOpen();

	ImGui::GetIO().MouseDrawCursor = isMainWindowOpen | isLogWindowOpen | isPaletteEditorWindowOpen
		| isUpdateNotifierWindowOpen | isDebugWindowOpen;

	DrawAllWindows();

	ImGui::Render();
}

void WindowManager::HandleButtons()
{
	if (!m_initialized)
	{
		return;
	}

	if (ImGui::IsKeyPressed(keyToggleHud) && g_gameVals.pIsHUDHidden)
	{
		*g_gameVals.pIsHUDHidden ^= 1;
	}

	if (ImGui::IsKeyPressed(keyToggleMainWindow))
	{
		m_windowContainer->GetWindow(WindowType_Main)->ToggleOpen();
	}
}

void WindowManager::DrawAllWindows() const
{
	for (const auto& window : m_windowContainer->GetWindows())
	{
		window.second->Update();
	}
}

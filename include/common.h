#pragma once

#include <windows.h>
#include <commctrl.h>
#include <tlhelp32.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <algorithm>
#include <shlwapi.h>
#include <memory>
#include <map>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shlwapi.lib")

// 前方宣言
class MinecraftBorderless;
class WindowManager;
class ProcessDetector;
class SettingsManager;
class TrayManager;

// 共通構造体
struct WindowInfo {
    LONG style;
    LONG exStyle;
    RECT rect;
};

struct GameProcess {
    DWORD pid;
    HWND hwnd;
    std::wstring processName;
    std::wstring windowTitle;
};

// 共通定数
namespace Constants {
    constexpr int AUTO_DETECT_INTERVAL = 3000; // ms
    constexpr int MIN_WINDOW_WIDTH = 100;
    constexpr int MIN_WINDOW_HEIGHT = 100;
    constexpr wchar_t APP_NAME[] = L"Minecraft ボーダーレス化ツール";
    constexpr wchar_t WINDOW_CLASS[] = L"MinecraftBorderlessClass";
}

// ユーティリティ関数
namespace Utils {
    std::wstring ToLower(const std::wstring& str);
    bool IsMinecraftProcess(const std::wstring& processName);
    bool IsValidGameWindow(HWND hwnd);
} 
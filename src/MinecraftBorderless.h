#pragma once

#include "../include/common.h"
#include "../include/resource.h"
#include "ProcessDetector.h"
#include "WindowManager.h"
#include "TrayManager.h"
#include "SettingsManager.h"

class MinecraftBorderless {
public:
    MinecraftBorderless();
    ~MinecraftBorderless();

    // 初期化
    bool Initialize(HINSTANCE hInstance);
    void Cleanup();
    
    // ウィンドウプロシージャ
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    // メイン機能
    void DetectMinecraft();
    void MakeBorderless();
    void RestoreWindow();
    void ToggleAutoMode();
    void AutoDetectMinecraft();
    void MinimizeToTray();
    void RestoreFromTray();
    void ToggleStartupMode();
    
    // UI更新
    void UpdateStatus(const std::wstring& status);
    void UpdateMinecraftStatus(const std::wstring& status);
    void UpdateUI();
    
    // アクセサ
    HWND GetMainWindow() const { return hMainWindow; }
    bool IsAutoMode() const { return autoMode; }

private:
    // ウィンドウハンドル
    HWND hMainWindow;
    HWND hBtnDetect, hBtnBorderless, hBtnRestore, hBtnMinimize;
    HWND hLblStatus, hLblMinecraft;
    HWND hChkAutoMode, hChkStartup;
    
    // 管理クラス
    std::unique_ptr<ProcessDetector> processDetector;
    std::unique_ptr<WindowManager> windowManager;
    std::unique_ptr<TrayManager> trayManager;
    std::unique_ptr<SettingsManager> settingsManager;
    
    // 状態
    GameProcess currentGame;
    bool autoMode;
    bool isBorderless;
    bool isMinimizedToTray;
    
    // 初期化ヘルパー
    bool InitializeWindow(HINSTANCE hInstance);
    bool CreateControls(HINSTANCE hInstance);
    void SetupFonts();
    
    // イベントハンドラ
    void OnCommand(WPARAM wParam, LPARAM lParam);
    void OnTimer(WPARAM wParam);
    void OnTrayIcon(WPARAM wParam, LPARAM lParam);
    void OnDestroy();
    
    // UI状態更新
    void EnableControls(bool detectEnabled, bool borderlessEnabled, bool restoreEnabled, bool minimizeEnabled);
    void UpdateCheckboxes();
}; 
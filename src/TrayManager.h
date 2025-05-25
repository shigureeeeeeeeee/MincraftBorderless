#pragma once

#include "../include/common.h"
#include "../include/resource.h"

class TrayManager {
public:
    TrayManager();
    ~TrayManager();

    // トレイアイコン管理
    bool SetupTrayIcon(HWND hwnd);
    bool CleanupTrayIcon();
    bool UpdateTrayIcon(const std::wstring& tooltip);
    bool ShowBalloonTip(const std::wstring& title, const std::wstring& message);
    
    // トレイメッセージ処理
    bool HandleTrayMessage(HWND hwnd, WPARAM wParam, LPARAM lParam);
    
    // 状態管理
    bool IsActive() const { return isActive; }

private:
    NOTIFYICONDATA nid;
    bool isActive;
    HWND parentWindow;
    
    void ShowContextMenu(HWND hwnd, POINT pt);
    HMENU CreateTrayContextMenu();
}; 
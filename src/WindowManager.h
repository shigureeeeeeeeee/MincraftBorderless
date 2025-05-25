#pragma once

#include "../include/common.h"

class WindowManager {
public:
    WindowManager();
    ~WindowManager();

    // ウィンドウ操作
    bool MakeBorderless(HWND hwnd);
    bool RestoreWindow(HWND hwnd);
    bool MinimizeToTray(HWND hwnd);
    bool RestoreFromTray(HWND hwnd);
    
    // ウィンドウ情報管理
    void SaveWindowInfo(HWND hwnd);
    WindowInfo GetSavedWindowInfo(HWND hwnd) const;
    bool HasSavedWindowInfo(HWND hwnd) const;
    
    // モニター関連
    HMONITOR GetPrimaryMonitor();
    HMONITOR GetMonitorFromWindow(HWND hwnd);
    RECT GetMonitorWorkArea(HMONITOR hMonitor);
    RECT GetMonitorRect(HMONITOR hMonitor);
    
    // ウィンドウ状態チェック
    bool IsBorderless(HWND hwnd) const;
    bool IsMinimized(HWND hwnd) const;

private:
    std::map<HWND, WindowInfo> savedWindowInfo;
    std::map<HWND, bool> minimizedWindows;
    
    void ApplyBorderlessStyle(HWND hwnd);
    void SetFullscreenPosition(HWND hwnd, HMONITOR hMonitor);
}; 
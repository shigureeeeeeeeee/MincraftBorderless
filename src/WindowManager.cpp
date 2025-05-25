#include "WindowManager.h"
#include <map>

WindowManager::WindowManager() {
}

WindowManager::~WindowManager() {
}

bool WindowManager::MakeBorderless(HWND hwnd) {
    if (!hwnd || !IsWindow(hwnd)) {
        return false;
    }
    
    // 現在のウィンドウ情報を保存
    SaveWindowInfo(hwnd);
    
    // ボーダーレススタイルを適用
    ApplyBorderlessStyle(hwnd);
    
    // フルスクリーン位置に設定
    HMONITOR hMonitor = GetMonitorFromWindow(hwnd);
    SetFullscreenPosition(hwnd, hMonitor);
    
    return true;
}

bool WindowManager::RestoreWindow(HWND hwnd) {
    if (!hwnd || !IsWindow(hwnd) || !HasSavedWindowInfo(hwnd)) {
        return false;
    }
    
    WindowInfo info = GetSavedWindowInfo(hwnd);
    
    // 元のウィンドウスタイルを復元
    SetWindowLong(hwnd, GWL_STYLE, info.style);
    SetWindowLong(hwnd, GWL_EXSTYLE, info.exStyle);
    
    // 元のサイズと位置を復元
    SetWindowPos(hwnd, nullptr,
        info.rect.left, info.rect.top,
        info.rect.right - info.rect.left,
        info.rect.bottom - info.rect.top,
        SWP_NOZORDER | SWP_FRAMECHANGED);
    
    // 保存された情報を削除
    savedWindowInfo.erase(hwnd);
    
    return true;
}

bool WindowManager::MinimizeToTray(HWND hwnd) {
    if (!hwnd || !IsWindow(hwnd)) {
        return false;
    }
    
    ShowWindow(hwnd, SW_HIDE);
    minimizedWindows[hwnd] = true;
    
    return true;
}

bool WindowManager::RestoreFromTray(HWND hwnd) {
    if (!hwnd || !IsWindow(hwnd)) {
        return false;
    }
    
    ShowWindow(hwnd, SW_RESTORE);
    minimizedWindows.erase(hwnd);
    
    return true;
}

void WindowManager::SaveWindowInfo(HWND hwnd) {
    if (!hwnd || !IsWindow(hwnd)) {
        return;
    }
    
    WindowInfo info;
    info.style = GetWindowLong(hwnd, GWL_STYLE);
    info.exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    GetWindowRect(hwnd, &info.rect);
    
    savedWindowInfo[hwnd] = info;
}

WindowInfo WindowManager::GetSavedWindowInfo(HWND hwnd) const {
    auto it = savedWindowInfo.find(hwnd);
    if (it != savedWindowInfo.end()) {
        return it->second;
    }
    return {}; // 空のWindowInfoを返す
}

bool WindowManager::HasSavedWindowInfo(HWND hwnd) const {
    return savedWindowInfo.find(hwnd) != savedWindowInfo.end();
}

HMONITOR WindowManager::GetPrimaryMonitor() {
    return MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY);
}

HMONITOR WindowManager::GetMonitorFromWindow(HWND hwnd) {
    return MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
}

RECT WindowManager::GetMonitorWorkArea(HMONITOR hMonitor) {
    MONITORINFO mi = { sizeof(mi) };
    GetMonitorInfo(hMonitor, &mi);
    return mi.rcWork;
}

RECT WindowManager::GetMonitorRect(HMONITOR hMonitor) {
    MONITORINFO mi = { sizeof(mi) };
    GetMonitorInfo(hMonitor, &mi);
    return mi.rcMonitor;
}

bool WindowManager::IsBorderless(HWND hwnd) const {
    if (!hwnd || !IsWindow(hwnd)) {
        return false;
    }
    
    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    return !(style & (WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU));
}

bool WindowManager::IsMinimized(HWND hwnd) const {
    auto it = minimizedWindows.find(hwnd);
    return it != minimizedWindows.end() && it->second;
}

void WindowManager::ApplyBorderlessStyle(HWND hwnd) {
    // ウィンドウスタイルを変更してボーダーを削除
    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
    SetWindowLong(hwnd, GWL_STYLE, style);
    
    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    exStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
    SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);
}

void WindowManager::SetFullscreenPosition(HWND hwnd, HMONITOR hMonitor) {
    RECT monitorRect = GetMonitorRect(hMonitor);
    
    SetWindowPos(hwnd, nullptr,
        monitorRect.left, monitorRect.top,
        monitorRect.right - monitorRect.left,
        monitorRect.bottom - monitorRect.top,
        SWP_NOZORDER | SWP_FRAMECHANGED);
} 
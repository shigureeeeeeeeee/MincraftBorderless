#include "TrayManager.h"

TrayManager::TrayManager() : isActive(false), parentWindow(nullptr) {
    ZeroMemory(&nid, sizeof(nid));
}

TrayManager::~TrayManager() {
    CleanupTrayIcon();
}

bool TrayManager::SetupTrayIcon(HWND hwnd) {
    if (isActive) {
        return true; // 既にアクティブ
    }
    
    parentWindow = hwnd;
    
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    strcpy_s(nid.szTip, "Minecraft Borderless Tool");
    
    bool result = Shell_NotifyIcon(NIM_ADD, &nid) != FALSE;
    if (result) {
        isActive = true;
    }
    
    return result;
}

bool TrayManager::CleanupTrayIcon() {
    if (!isActive) {
        return true;
    }
    
    bool result = Shell_NotifyIcon(NIM_DELETE, &nid) != FALSE;
    if (result) {
        isActive = false;
        parentWindow = nullptr;
    }
    
    return result;
}

bool TrayManager::UpdateTrayIcon(const std::wstring& tooltip) {
    if (!isActive) {
        return false;
    }
    
    // Unicode文字列をANSI文字列に変換
    std::string ansiTooltip;
    ansiTooltip.resize(tooltip.length());
    WideCharToMultiByte(CP_ACP, 0, tooltip.c_str(), -1, &ansiTooltip[0], ansiTooltip.size(), nullptr, nullptr);
    
    strcpy_s(nid.szTip, ansiTooltip.c_str());
    return Shell_NotifyIcon(NIM_MODIFY, &nid) != FALSE;
}

bool TrayManager::ShowBalloonTip(const std::wstring& title, const std::wstring& message) {
    if (!isActive) {
        return false;
    }
    
    // Unicode文字列をANSI文字列に変換
    std::string ansiTitle, ansiMessage;
    ansiTitle.resize(title.length() * 2);
    ansiMessage.resize(message.length() * 2);
    
    WideCharToMultiByte(CP_ACP, 0, title.c_str(), -1, &ansiTitle[0], ansiTitle.size(), nullptr, nullptr);
    WideCharToMultiByte(CP_ACP, 0, message.c_str(), -1, &ansiMessage[0], ansiMessage.size(), nullptr, nullptr);
    
    nid.uFlags |= NIF_INFO;
    strcpy_s(nid.szInfoTitle, ansiTitle.c_str());
    strcpy_s(nid.szInfo, ansiMessage.c_str());
    nid.dwInfoFlags = NIIF_INFO;
    nid.uTimeout = 3000; // 3秒
    
    bool result = Shell_NotifyIcon(NIM_MODIFY, &nid) != FALSE;
    
    // フラグをリセット
    nid.uFlags &= ~NIF_INFO;
    
    return result;
}

bool TrayManager::HandleTrayMessage(HWND hwnd, WPARAM wParam, LPARAM lParam) {
    if (!isActive || wParam != nid.uID) {
        return false;
    }
    
    switch (lParam) {
    case WM_LBUTTONDOWN:
        // 左クリック - ウィンドウを復元
        PostMessage(hwnd, WM_COMMAND, IDM_RESTORE, 0);
        return true;
        
    case WM_RBUTTONDOWN:
        // 右クリック - コンテキストメニューを表示
        POINT pt;
        GetCursorPos(&pt);
        ShowContextMenu(hwnd, pt);
        return true;
    }
    
    return false;
}

void TrayManager::ShowContextMenu(HWND hwnd, POINT pt) {
    HMENU hMenu = CreateTrayContextMenu();
    if (!hMenu) {
        return;
    }
    
    // フォアグラウンドウィンドウに設定（メニューが正しく動作するため）
    SetForegroundWindow(hwnd);
    
    int cmd = TrackPopupMenu(hMenu, 
        TPM_RETURNCMD | TPM_RIGHTBUTTON, 
        pt.x, pt.y, 0, hwnd, nullptr);
    
    if (cmd != 0) {
        PostMessage(hwnd, WM_COMMAND, cmd, 0);
    }
    
    DestroyMenu(hMenu);
}

HMENU TrayManager::CreateTrayContextMenu() {
    HMENU hMenu = CreatePopupMenu();
    if (!hMenu) {
        return nullptr;
    }
    
    AppendMenuW(hMenu, MF_STRING, IDM_RESTORE, L"復元");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, IDM_EXIT, L"終了");
    
    return hMenu;
} 
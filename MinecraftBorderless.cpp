#include <windows.h>
#include <commctrl.h>
#include <tlhelp32.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <algorithm>
#include <shlwapi.h>
#include "resource.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shlwapi.lib")

class MinecraftBorderless {
public:
    HWND hMainWindow;
    HWND hChkAutoMode;
    
private:
    HWND hBtnDetect, hBtnBorderless, hBtnRestore, hBtnMinimize;
    HWND hLblStatus, hLblMinecraft;
    HWND hChkStartup;
    HWND hMinecraftWindow;
    DWORD minecraftPID;
    
    // 元のウィンドウ情報を保存
    struct WindowInfo {
        LONG style;
        LONG exStyle;
        RECT rect;
    } originalInfo;
    
    bool isBorderless = false;
    bool autoMode = false;
    bool isMinimizedToTray = false;
    bool startupMode = false;
    NOTIFYICONDATA nid;
    
public:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    bool InitializeWindow(HINSTANCE hInstance);
    void DetectMinecraft();
    void MakeBorderless();
    void RestoreWindow();
    void UpdateStatus(const std::wstring& status);
    void UpdateMinecraftStatus(const std::wstring& status);
    void ToggleAutoMode();
    void AutoDetectMinecraft();
    void MinimizeToTray();
    void RestoreFromTray();
    void SetupTrayIcon();
    void CleanupTrayIcon();
    void ToggleStartupMode();
    void SetStartupRegistration(bool enable);
    bool IsStartupRegistered();
    void LoadSettings();
    void SaveSettings();
    
    static std::vector<DWORD> FindMinecraftProcesses();
    static bool IsMinecraftWindow(HWND hwnd);
    static std::wstring GetWindowText(HWND hwnd);
    static std::wstring GetProcessName(DWORD pid);
};

// グローバルインスタンス
MinecraftBorderless* g_pApp = nullptr;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    InitCommonControls();
    
    // コマンドライン引数をチェック
    bool startupLaunch = false;
    std::string cmdLine = lpCmdLine;
    if (cmdLine.find("-startup") != std::string::npos) {
        startupLaunch = true;
        nCmdShow = SW_HIDE; // スタートアップ時は非表示で開始
    }
    
    MinecraftBorderless app;
    g_pApp = &app;
    
    if (!app.InitializeWindow(hInstance)) {
        MessageBoxW(nullptr, L"ウィンドウの初期化に失敗しました。", L"エラー", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    // 設定を読み込み
    app.LoadSettings();
    
    // スタートアップ起動の場合の処理
    if (startupLaunch) {
        // 自動モードを有効にする
        SendMessage(app.hChkAutoMode, BM_SETCHECK, BST_CHECKED, 0);
        app.ToggleAutoMode();
        
        // システムトレイに最小化
        app.SetupTrayIcon();
    } else {
        ShowWindow(app.hMainWindow, nCmdShow);
        UpdateWindow(app.hMainWindow);
    }
    
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}

bool MinecraftBorderless::InitializeWindow(HINSTANCE hInstance) {
    const wchar_t* className = L"MinecraftBorderlessClass";
    
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    
    if (!RegisterClassW(&wc)) {
        return false;
    }
    
    hMainWindow = CreateWindowExW(
        0,
        className,
        L"Minecraft ボーダーレス化ツール",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 420, 380,
        nullptr, nullptr, hInstance, nullptr
    );
    
    if (!hMainWindow) {
        return false;
    }
    
    // コントロールを作成
    hBtnDetect = CreateWindowW(
        L"BUTTON", L"Minecraftを検出",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        20, 20, 150, 30,
        hMainWindow, (HMENU)IDC_BTN_DETECT, hInstance, nullptr
    );
    
    hBtnBorderless = CreateWindowW(
        L"BUTTON", L"ボーダーレス化",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | WS_DISABLED,
        20, 60, 150, 30,
        hMainWindow, (HMENU)IDC_BTN_BORDERLESS, hInstance, nullptr
    );
    
    hBtnRestore = CreateWindowW(
        L"BUTTON", L"元に戻す",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | WS_DISABLED,
        20, 100, 150, 30,
        hMainWindow, (HMENU)IDC_BTN_RESTORE, hInstance, nullptr
    );
    
    hBtnMinimize = CreateWindowW(
        L"BUTTON", L"システムトレイに最小化",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | WS_DISABLED,
        20, 140, 180, 30,
        hMainWindow, (HMENU)IDC_BTN_MINIMIZE, hInstance, nullptr
    );
    
    hLblStatus = CreateWindowW(
        L"STATIC", L"ステータス: 待機中",
        WS_VISIBLE | WS_CHILD,
        20, 185, 350, 20,
        hMainWindow, (HMENU)IDC_LBL_STATUS, hInstance, nullptr
    );
    
    hLblMinecraft = CreateWindowW(
        L"STATIC", L"Minecraft: 未検出",
        WS_VISIBLE | WS_CHILD,
        20, 210, 350, 20,
        hMainWindow, (HMENU)IDC_LBL_MINECRAFT, hInstance, nullptr
    );
    
    hChkAutoMode = CreateWindowW(
        L"BUTTON", L"自動モード",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        20, 245, 150, 25,
        hMainWindow, (HMENU)IDC_CHK_AUTO_MODE, hInstance, nullptr
    );
    
    hChkStartup = CreateWindowW(
        L"BUTTON", L"Windows起動時に自動実行",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        20, 275, 200, 25,
        hMainWindow, (HMENU)IDC_CHK_STARTUP, hInstance, nullptr
    );
    
    // フォント設定
    HFONT hFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"MS UI Gothic");
    
    SendMessage(hBtnDetect, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hBtnBorderless, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hBtnRestore, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hBtnMinimize, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hLblStatus, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hLblMinecraft, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hChkAutoMode, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hChkStartup, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    return true;
}

LRESULT CALLBACK MinecraftBorderless::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_COMMAND:
        if (g_pApp) {
            switch (LOWORD(wParam)) {
            case IDC_BTN_DETECT:
                g_pApp->DetectMinecraft();
                break;
            case IDC_BTN_BORDERLESS:
                g_pApp->MakeBorderless();
                break;
            case IDC_BTN_RESTORE:
                g_pApp->RestoreWindow();
                break;
            case IDC_BTN_MINIMIZE:
                g_pApp->MinimizeToTray();
                break;
            case IDC_CHK_AUTO_MODE:
                g_pApp->ToggleAutoMode();
                break;
            case IDC_CHK_STARTUP:
                g_pApp->ToggleStartupMode();
                break;
            }
        }
        break;
        
    case WM_TIMER:
        if (wParam == TIMER_AUTO_DETECT && g_pApp) {
            g_pApp->AutoDetectMinecraft();
        }
        break;
        
    case WM_TRAYICON:
        if (lParam == WM_LBUTTONDOWN) {
            if (g_pApp) {
                g_pApp->RestoreFromTray();
            }
        } else if (lParam == WM_RBUTTONDOWN) {
            if (g_pApp) {
                // コンテキストメニューを表示
                POINT pt;
                GetCursorPos(&pt);
                HMENU hMenu = CreatePopupMenu();
                AppendMenuW(hMenu, MF_STRING, IDM_RESTORE, L"復元");
                AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
                AppendMenuW(hMenu, MF_STRING, IDM_EXIT, L"終了");
                
                SetForegroundWindow(hwnd);
                int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, nullptr);
                
                if (cmd == IDM_RESTORE) {
                    g_pApp->RestoreFromTray();
                } else if (cmd == IDM_EXIT) {
                    PostMessage(hwnd, WM_CLOSE, 0, 0);
                }
                
                DestroyMenu(hMenu);
            }
        }
        break;
        
    case WM_DESTROY:
        if (g_pApp) {
            g_pApp->CleanupTrayIcon();
            KillTimer(hwnd, TIMER_AUTO_DETECT);
        }
        PostQuitMessage(0);
        break;
        
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

void MinecraftBorderless::DetectMinecraft() {
    auto processes = FindMinecraftProcesses();
    
    if (processes.empty()) {
        UpdateMinecraftStatus(L"Minecraft: 未検出");
        UpdateStatus(L"ステータス: Minecraftが見つかりません");
        EnableWindow(hBtnBorderless, FALSE);
        EnableWindow(hBtnRestore, FALSE);
        MessageBoxW(hMainWindow, 
            L"Minecraftが見つかりません。\nMinecraftを起動してから再試行してください。",
            L"検出エラー", MB_OK | MB_ICONWARNING);
        return;
    }
    
    // 各プロセスのウィンドウを検索
    HWND foundWindow = nullptr;
    DWORD foundPID = 0;
    
    for (DWORD pid : processes) {
        // プロセスのウィンドウを列挙
        struct EnumData {
            DWORD targetPID;
            HWND* pFoundWindow;
        };
        
        EnumData enumData = { pid, &foundWindow };
        
        EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
            DWORD pid;
            GetWindowThreadProcessId(hwnd, &pid);
            
            auto* data = reinterpret_cast<EnumData*>(lParam);
            
            if (pid == data->targetPID && IsWindowVisible(hwnd) && GetParent(hwnd) == nullptr) {
                // ウィンドウタイトルをチェック
                wchar_t title[256];
                GetWindowTextW(hwnd, title, sizeof(title) / sizeof(wchar_t));
                std::wstring windowTitle = title;
                std::transform(windowTitle.begin(), windowTitle.end(), windowTitle.begin(), ::towlower);
                
                // Minecraftかどうかチェック（より柔軟な条件）
                if (windowTitle.find(L"minecraft") != std::wstring::npos ||
                    windowTitle.find(L"mc") != std::wstring::npos ||
                    windowTitle.length() == 0) { // タイトルなしの場合も許可
                    
                    // ウィンドウのサイズが妥当かチェック（最小サイズ）
                    RECT rect;
                    GetWindowRect(hwnd, &rect);
                    int width = rect.right - rect.left;
                    int height = rect.bottom - rect.top;
                    
                    if (width > 100 && height > 100) { // 最小サイズチェック
                        *(data->pFoundWindow) = hwnd;
                        return FALSE; // 見つかったので停止
                    }
                }
            }
            return TRUE;
        }, reinterpret_cast<LPARAM>(&enumData));
        
        if (foundWindow) {
            foundPID = pid;
            break;
        }
    }
    
    if (foundWindow) {
        hMinecraftWindow = foundWindow;
        minecraftPID = foundPID;
        
        // 元のウィンドウ情報を保存
        originalInfo.style = GetWindowLong(hMinecraftWindow, GWL_STYLE);
        originalInfo.exStyle = GetWindowLong(hMinecraftWindow, GWL_EXSTYLE);
        GetWindowRect(hMinecraftWindow, &originalInfo.rect);
        
        // ウィンドウタイトルを取得
        wchar_t title[256];
        GetWindowTextW(hMinecraftWindow, title, sizeof(title) / sizeof(wchar_t));
        std::wstring windowTitle = title;
        if (windowTitle.empty()) {
            windowTitle = L"(タイトルなし)";
        }
        
        std::wstring processName = GetProcessName(minecraftPID);
        UpdateMinecraftStatus(L"Minecraft: 検出済み (" + processName + L" - " + windowTitle + L")");
        UpdateStatus(L"ステータス: Minecraft検出完了");
        EnableWindow(hBtnBorderless, TRUE);
    } else {
        UpdateMinecraftStatus(L"Minecraft: ウィンドウが見つかりません");
        UpdateStatus(L"ステータス: Minecraftプロセスは見つかりましたが、ウィンドウが特定できません");
        MessageBoxW(hMainWindow, 
            L"Minecraftプロセスは見つかりましたが、メインウィンドウを特定できません。\nMinecraftが完全に起動しているか確認してください。",
            L"検出エラー", MB_OK | MB_ICONWARNING);
    }
}

void MinecraftBorderless::MakeBorderless() {
    if (!hMinecraftWindow || !IsWindow(hMinecraftWindow)) {
        MessageBoxW(hMainWindow, L"Minecraftが検出されていません。", L"エラー", MB_OK | MB_ICONWARNING);
        return;
    }
    
    // ウィンドウスタイルを変更してボーダーを削除
    LONG style = GetWindowLong(hMinecraftWindow, GWL_STYLE);
    style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
    SetWindowLong(hMinecraftWindow, GWL_STYLE, style);
    
    LONG exStyle = GetWindowLong(hMinecraftWindow, GWL_EXSTYLE);
    exStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
    SetWindowLong(hMinecraftWindow, GWL_EXSTYLE, exStyle);
    
    // スクリーンサイズを取得
    HMONITOR hMonitor = MonitorFromWindow(hMinecraftWindow, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { sizeof(mi) };
    GetMonitorInfo(hMonitor, &mi);
    
    // 全画面に設定
    SetWindowPos(hMinecraftWindow, nullptr,
        mi.rcMonitor.left, mi.rcMonitor.top,
        mi.rcMonitor.right - mi.rcMonitor.left,
        mi.rcMonitor.bottom - mi.rcMonitor.top,
        SWP_NOZORDER | SWP_FRAMECHANGED);
    
    isBorderless = true;
    UpdateStatus(L"ステータス: ボーダーレス化完了");
    EnableWindow(hBtnBorderless, FALSE);
    EnableWindow(hBtnRestore, TRUE);
    
    if (!autoMode) {
        MessageBoxW(hMainWindow, L"Minecraftをボーダーレス化しました！", L"完了", MB_OK | MB_ICONINFORMATION);
    }
}

void MinecraftBorderless::RestoreWindow() {
    if (!hMinecraftWindow || !IsWindow(hMinecraftWindow)) {
        MessageBoxW(hMainWindow, L"Minecraftが検出されていません。", L"エラー", MB_OK | MB_ICONWARNING);
        return;
    }
    
    // 元のウィンドウスタイルを復元
    SetWindowLong(hMinecraftWindow, GWL_STYLE, originalInfo.style);
    SetWindowLong(hMinecraftWindow, GWL_EXSTYLE, originalInfo.exStyle);
    
    // 元のサイズと位置を復元
    SetWindowPos(hMinecraftWindow, nullptr,
        originalInfo.rect.left, originalInfo.rect.top,
        originalInfo.rect.right - originalInfo.rect.left,
        originalInfo.rect.bottom - originalInfo.rect.top,
        SWP_NOZORDER | SWP_FRAMECHANGED);
    
    isBorderless = false;
    UpdateStatus(L"ステータス: ウィンドウを復元しました");
    EnableWindow(hBtnBorderless, TRUE);
    EnableWindow(hBtnRestore, FALSE);
    
    if (!autoMode) {
        MessageBoxW(hMainWindow, L"Minecraftウィンドウを元に戻しました。", L"完了", MB_OK | MB_ICONINFORMATION);
    }
}

void MinecraftBorderless::UpdateStatus(const std::wstring& status) {
    SetWindowTextW(hLblStatus, status.c_str());
}

void MinecraftBorderless::UpdateMinecraftStatus(const std::wstring& status) {
    SetWindowTextW(hLblMinecraft, status.c_str());
}

void MinecraftBorderless::ToggleAutoMode() {
    autoMode = SendMessage(hChkAutoMode, BM_GETCHECK, 0, 0) == BST_CHECKED;
    
    if (autoMode) {
        // タイマーを開始（3秒間隔で監視）
        SetTimer(hMainWindow, TIMER_AUTO_DETECT, 3000, nullptr);
        UpdateStatus(L"ステータス: 自動モード ON - Minecraftを監視中...");
        EnableWindow(hBtnMinimize, hMinecraftWindow && IsWindow(hMinecraftWindow) ? TRUE : FALSE);
    } else {
        // タイマーを停止
        KillTimer(hMainWindow, TIMER_AUTO_DETECT);
        UpdateStatus(L"ステータス: 自動モード OFF");
        EnableWindow(hBtnMinimize, FALSE);
    }
}

void MinecraftBorderless::AutoDetectMinecraft() {
    if (!autoMode) return;
    
    // 既にMinecraftが検出されている場合はスキップ
    if (hMinecraftWindow && IsWindow(hMinecraftWindow)) {
        // プロセスがまだ生きているかチェック
        DWORD exitCode;
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, minecraftPID);
        if (hProcess) {
            GetExitCodeProcess(hProcess, &exitCode);
            CloseHandle(hProcess);
            
            if (exitCode == STILL_ACTIVE && !isBorderless) {
                // Minecraftが起動中で、まだボーダーレス化されていない場合
                UpdateStatus(L"ステータス: Minecraft検出 - 自動ボーダーレス化実行中...");
                MakeBorderless();
                EnableWindow(hBtnMinimize, TRUE);
                return;
            } else if (exitCode != STILL_ACTIVE) {
                // プロセスが終了している場合
                hMinecraftWindow = nullptr;
                minecraftPID = 0;
                isBorderless = false;
                UpdateMinecraftStatus(L"Minecraft: 未検出");
                UpdateStatus(L"ステータス: 自動モード ON - Minecraftを監視中...");
                EnableWindow(hBtnBorderless, FALSE);
                EnableWindow(hBtnRestore, FALSE);
                EnableWindow(hBtnMinimize, FALSE);
            }
        }
        return;
    }
    
    // Minecraftを自動検出
    auto processes = FindMinecraftProcesses();
    if (processes.empty()) {
        return; // 見つからない場合は何もしない
    }
    
    // 各プロセスのウィンドウを検索
    HWND foundWindow = nullptr;
    DWORD foundPID = 0;
    
    for (DWORD pid : processes) {
        struct EnumData {
            DWORD targetPID;
            HWND* pFoundWindow;
        };
        
        EnumData enumData = { pid, &foundWindow };
        
        EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
            DWORD pid;
            GetWindowThreadProcessId(hwnd, &pid);
            
            auto* data = reinterpret_cast<EnumData*>(lParam);
            
            if (pid == data->targetPID && IsWindowVisible(hwnd) && GetParent(hwnd) == nullptr) {
                wchar_t title[256];
                GetWindowTextW(hwnd, title, sizeof(title) / sizeof(wchar_t));
                std::wstring windowTitle = title;
                std::transform(windowTitle.begin(), windowTitle.end(), windowTitle.begin(), ::towlower);
                
                if (windowTitle.find(L"minecraft") != std::wstring::npos ||
                    windowTitle.find(L"mc") != std::wstring::npos ||
                    windowTitle.length() == 0) {
                    
                    RECT rect;
                    GetWindowRect(hwnd, &rect);
                    int width = rect.right - rect.left;
                    int height = rect.bottom - rect.top;
                    
                    if (width > 100 && height > 100) {
                        *(data->pFoundWindow) = hwnd;
                        return FALSE;
                    }
                }
            }
            return TRUE;
        }, reinterpret_cast<LPARAM>(&enumData));
        
        if (foundWindow) {
            foundPID = pid;
            break;
        }
    }
    
    if (foundWindow) {
        hMinecraftWindow = foundWindow;
        minecraftPID = foundPID;
        
        // 元のウィンドウ情報を保存
        originalInfo.style = GetWindowLong(hMinecraftWindow, GWL_STYLE);
        originalInfo.exStyle = GetWindowLong(hMinecraftWindow, GWL_EXSTYLE);
        GetWindowRect(hMinecraftWindow, &originalInfo.rect);
        
        std::wstring processName = GetProcessName(minecraftPID);
        UpdateMinecraftStatus(L"Minecraft: 自動検出済み (" + processName + L")");
        UpdateStatus(L"ステータス: Minecraft検出 - 自動ボーダーレス化実行中...");
        
        // 自動的にボーダーレス化を実行
        MakeBorderless();
        EnableWindow(hBtnBorderless, FALSE);
        EnableWindow(hBtnRestore, TRUE);
        EnableWindow(hBtnMinimize, TRUE);
    }
}

void MinecraftBorderless::MinimizeToTray() {
    if (!hMinecraftWindow || !IsWindow(hMinecraftWindow)) {
        MessageBoxW(hMainWindow, L"Minecraftが検出されていません。", L"エラー", MB_OK | MB_ICONWARNING);
        return;
    }
    
    // ウィンドウを非表示にする
    ShowWindow(hMinecraftWindow, SW_HIDE);
    isMinimizedToTray = true;
    UpdateStatus(L"ステータス: ウィンドウをシステムトレイに最小化しました");
    EnableWindow(hBtnBorderless, FALSE);
    EnableWindow(hBtnRestore, FALSE);
    EnableWindow(hBtnMinimize, FALSE);
    
    SetupTrayIcon();
}

void MinecraftBorderless::RestoreFromTray() {
    if (isMinimizedToTray) {
        // ウィンドウを表示する
        ShowWindow(hMinecraftWindow, SW_RESTORE);
        isMinimizedToTray = false;
        UpdateStatus(L"ステータス: ウィンドウを元に戻しました");
        EnableWindow(hBtnBorderless, TRUE);
        EnableWindow(hBtnRestore, TRUE);
        EnableWindow(hBtnMinimize, TRUE);
        
        CleanupTrayIcon();
    }
}

void MinecraftBorderless::SetupTrayIcon() {
    // トレイアイコンの設定
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hMainWindow;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    strcpy_s(nid.szTip, "Minecraft Borderless Tool");
    
    Shell_NotifyIcon(NIM_ADD, &nid);
}

void MinecraftBorderless::CleanupTrayIcon() {
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

std::vector<DWORD> MinecraftBorderless::FindMinecraftProcesses() {
    std::vector<DWORD> processes;
    
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return processes;
    }
    
    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    
    if (Process32FirstW(hSnapshot, &pe32)) {
        do {
            std::wstring processName = pe32.szExeFile;
            std::transform(processName.begin(), processName.end(), processName.begin(), ::towlower);
            
            // より広範囲なMinecraft関連プロセスを検出
            if (processName.find(L"minecraft") != std::wstring::npos ||
                processName.find(L"javaw") != std::wstring::npos ||
                processName.find(L"java") != std::wstring::npos ||
                processName.find(L"mc") != std::wstring::npos) {
                
                // Javaプロセスの場合、コマンドラインもチェック
                if (processName.find(L"java") != std::wstring::npos) {
                    // すべてのJavaプロセスを一旦追加（後でウィンドウタイトルで絞り込み）
                    processes.push_back(pe32.th32ProcessID);
                } else {
                    // 明らかにMinecraft関連の場合は直接追加
                    processes.push_back(pe32.th32ProcessID);
                }
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }
    
    CloseHandle(hSnapshot);
    return processes;
}

std::wstring MinecraftBorderless::GetProcessName(DWORD pid) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return L"Unknown";
    }
    
    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    
    if (Process32FirstW(hSnapshot, &pe32)) {
        do {
            if (pe32.th32ProcessID == pid) {
                CloseHandle(hSnapshot);
                return pe32.szExeFile;
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }
    
    CloseHandle(hSnapshot);
    return L"Unknown";
}

void MinecraftBorderless::ToggleStartupMode() {
    startupMode = SendMessage(hChkStartup, BM_GETCHECK, 0, 0) == BST_CHECKED;
    SetStartupRegistration(startupMode);
    SaveSettings();
}

void MinecraftBorderless::SetStartupRegistration(bool enable) {
    const wchar_t* keyPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
    const wchar_t* valueName = L"MinecraftBorderless";
    
    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, keyPath, 0, KEY_WRITE, &hKey);
    
    if (result != ERROR_SUCCESS) {
        MessageBoxW(hMainWindow, L"レジストリキーのオープンに失敗しました。", L"エラー", MB_OK | MB_ICONERROR);
        return;
    }
    
    if (enable) {
        // 実行ファイルのフルパスを取得
        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        
        // スタートアップ時にトレイに最小化して起動するオプションを追加
        std::wstring commandLine = L"\"" + std::wstring(exePath) + L"\" -startup";
        
        result = RegSetValueExW(hKey, valueName, 0, REG_SZ, 
            (const BYTE*)commandLine.c_str(), 
            (commandLine.length() + 1) * sizeof(wchar_t));
        
        if (result == ERROR_SUCCESS) {
            UpdateStatus(L"ステータス: Windows起動時自動実行 ON");
        } else {
            MessageBoxW(hMainWindow, L"レジストリ値の設定に失敗しました。", L"エラー", MB_OK | MB_ICONERROR);
        }
    } else {
        result = RegDeleteValueW(hKey, valueName);
        
        if (result == ERROR_SUCCESS || result == ERROR_FILE_NOT_FOUND) {
            UpdateStatus(L"ステータス: Windows起動時自動実行 OFF");
        } else {
            MessageBoxW(hMainWindow, L"レジストリ値の削除に失敗しました。", L"エラー", MB_OK | MB_ICONERROR);
        }
    }
    
    RegCloseKey(hKey);
}

bool MinecraftBorderless::IsStartupRegistered() {
    const wchar_t* keyPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
    const wchar_t* valueName = L"MinecraftBorderless";
    
    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, keyPath, 0, KEY_READ, &hKey);
    
    if (result != ERROR_SUCCESS) {
        return false;
    }
    
    DWORD dataSize = 0;
    result = RegQueryValueExW(hKey, valueName, nullptr, nullptr, nullptr, &dataSize);
    
    RegCloseKey(hKey);
    return (result == ERROR_SUCCESS);
}

void MinecraftBorderless::LoadSettings() {
    // スタートアップ設定の状態をチェックボックスに反映
    bool isRegistered = IsStartupRegistered();
    SendMessage(hChkStartup, BM_SETCHECK, isRegistered ? BST_CHECKED : BST_UNCHECKED, 0);
    startupMode = isRegistered;
}

void MinecraftBorderless::SaveSettings() {
    // 現在の設定を保存（現在は自動的に保存されるため、特別な処理は不要）
    // 将来的には設定ファイルに保存する機能を追加可能
} 
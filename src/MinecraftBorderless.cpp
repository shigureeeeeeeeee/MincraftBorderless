#include "MinecraftBorderless.h"

// 外部からアクセス可能なグローバルインスタンス
extern MinecraftBorderless* g_pApp;

MinecraftBorderless::MinecraftBorderless() 
    : hMainWindow(nullptr)
    , hBtnDetect(nullptr), hBtnBorderless(nullptr), hBtnRestore(nullptr), hBtnMinimize(nullptr)
    , hLblStatus(nullptr), hLblMinecraft(nullptr)
    , hChkAutoMode(nullptr), hChkStartup(nullptr)
    , autoMode(false), isBorderless(false), isMinimizedToTray(false)
{
    // 管理クラスのインスタンスを作成
    processDetector = std::make_unique<ProcessDetector>();
    windowManager = std::make_unique<WindowManager>();
    trayManager = std::make_unique<TrayManager>();
    settingsManager = std::make_unique<SettingsManager>();
}

MinecraftBorderless::~MinecraftBorderless() {
    Cleanup();
}

bool MinecraftBorderless::Initialize(HINSTANCE hInstance) {
    // ウィンドウクラスの登録とウィンドウ作成
    if (!InitializeWindow(hInstance)) {
        return false;
    }
    
    // コントロールの作成
    if (!CreateControls(hInstance)) {
        return false;
    }
    
    // フォントの設定
    SetupFonts();
    
    // 設定の読み込み
    settingsManager->LoadSettings();
    
    // UIの初期状態を設定
    UpdateUI();
    
    return true;
}

void MinecraftBorderless::Cleanup() {
    // タイマーを停止
    if (hMainWindow) {
        KillTimer(hMainWindow, TIMER_AUTO_DETECT);
    }
    
    // トレイアイコンをクリーンアップ
    if (trayManager) {
        trayManager->CleanupTrayIcon();
    }
    
    // 設定を保存
    if (settingsManager) {
        settingsManager->SaveSettings();
    }
}

LRESULT CALLBACK MinecraftBorderless::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_COMMAND:
        if (g_pApp) {
            g_pApp->OnCommand(wParam, lParam);
        }
        break;
        
    case WM_TIMER:
        if (g_pApp) {
            g_pApp->OnTimer(wParam);
        }
        break;
        
    case WM_TRAYICON:
        if (g_pApp) {
            g_pApp->OnTrayIcon(wParam, lParam);
        }
        break;
        
    case WM_DESTROY:
        if (g_pApp) {
            g_pApp->OnDestroy();
        }
        break;
        
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

void MinecraftBorderless::DetectMinecraft() {
    currentGame = processDetector->FindMinecraftWindow();
    
    if (currentGame.hwnd == nullptr) {
        UpdateMinecraftStatus(L"Minecraft: 未検出");
        UpdateStatus(L"ステータス: Minecraftが見つかりません");
        EnableControls(true, false, false, false);
        
        if (!autoMode) {
            MessageBoxW(hMainWindow, 
                L"Minecraftが見つかりません。\nMinecraftを起動してから再試行してください。",
                L"検出エラー", MB_OK | MB_ICONWARNING);
        }
        return;
    }
    
    // ウィンドウタイトルの表示用文字列を作成
    std::wstring displayTitle = currentGame.windowTitle;
    if (displayTitle.empty()) {
        displayTitle = L"(タイトルなし)";
    }
    
    std::wstring statusText = L"Minecraft: 検出済み (" + currentGame.processName + L" - " + displayTitle + L")";
    UpdateMinecraftStatus(statusText);
    UpdateStatus(L"ステータス: Minecraft検出完了");
    
    // ボタンの状態を更新
    bool isCurrentlyBorderless = windowManager->IsBorderless(currentGame.hwnd);
    EnableControls(true, !isCurrentlyBorderless, isCurrentlyBorderless, autoMode);
    
    isBorderless = isCurrentlyBorderless;
}

void MinecraftBorderless::MakeBorderless() {
    if (currentGame.hwnd == nullptr || !IsWindow(currentGame.hwnd)) {
        MessageBoxW(hMainWindow, L"Minecraftが検出されていません。", L"エラー", MB_OK | MB_ICONWARNING);
        return;
    }
    
    if (windowManager->MakeBorderless(currentGame.hwnd)) {
        isBorderless = true;
        UpdateStatus(L"ステータス: ボーダーレス化完了");
        EnableControls(true, false, true, autoMode);
        
        if (!autoMode) {
            MessageBoxW(hMainWindow, L"Minecraftをボーダーレス化しました！", L"完了", MB_OK | MB_ICONINFORMATION);
        }
        
        // 通知を表示
        if (settingsManager->GetEnableNotifications() && trayManager->IsActive()) {
            trayManager->ShowBalloonTip(L"ボーダーレス化完了", L"Minecraftをボーダーレス化しました");
        }
    } else {
        UpdateStatus(L"ステータス: ボーダーレス化に失敗");
        MessageBoxW(hMainWindow, L"ボーダーレス化に失敗しました。", L"エラー", MB_OK | MB_ICONERROR);
    }
}

void MinecraftBorderless::RestoreWindow() {
    if (currentGame.hwnd == nullptr || !IsWindow(currentGame.hwnd)) {
        MessageBoxW(hMainWindow, L"Minecraftが検出されていません。", L"エラー", MB_OK | MB_ICONWARNING);
        return;
    }
    
    if (windowManager->RestoreWindow(currentGame.hwnd)) {
        isBorderless = false;
        UpdateStatus(L"ステータス: ウィンドウを復元しました");
        EnableControls(true, true, false, autoMode);
        
        if (!autoMode) {
            MessageBoxW(hMainWindow, L"Minecraftウィンドウを元に戻しました。", L"完了", MB_OK | MB_ICONINFORMATION);
        }
        
        // 通知を表示
        if (settingsManager->GetEnableNotifications() && trayManager->IsActive()) {
            trayManager->ShowBalloonTip(L"復元完了", L"Minecraftウィンドウを元に戻しました");
        }
    } else {
        UpdateStatus(L"ステータス: 復元に失敗");
        MessageBoxW(hMainWindow, L"ウィンドウの復元に失敗しました。", L"エラー", MB_OK | MB_ICONERROR);
    }
}

void MinecraftBorderless::ToggleAutoMode() {
    autoMode = SendMessage(hChkAutoMode, BM_GETCHECK, 0, 0) == BST_CHECKED;
    settingsManager->SetAutoMode(autoMode);
    
    if (autoMode) {
        // タイマーを開始
        SetTimer(hMainWindow, TIMER_AUTO_DETECT, settingsManager->GetAutoDetectInterval(), nullptr);
        UpdateStatus(L"ステータス: 自動モード ON - Minecraftを監視中...");
        EnableControls(true, false, false, true);
    } else {
        // タイマーを停止
        KillTimer(hMainWindow, TIMER_AUTO_DETECT);
        UpdateStatus(L"ステータス: 自動モード OFF");
        
        // ボタンの状態を更新
        bool hasGame = (currentGame.hwnd != nullptr && IsWindow(currentGame.hwnd));
        EnableControls(true, hasGame && !isBorderless, hasGame && isBorderless, false);
    }
}

void MinecraftBorderless::AutoDetectMinecraft() {
    if (!autoMode) return;
    
    // 既にMinecraftが検出されている場合
    if (currentGame.hwnd && IsWindow(currentGame.hwnd)) {
        // プロセスがまだ生きているかチェック
        if (processDetector->IsProcessRunning(currentGame.pid)) {
            if (!isBorderless) {
                // まだボーダーレス化されていない場合は実行
                UpdateStatus(L"ステータス: Minecraft検出 - 自動ボーダーレス化実行中...");
                MakeBorderless();
            }
            return;
        } else {
            // プロセスが終了している場合はリセット
            currentGame = {};
            isBorderless = false;
            UpdateMinecraftStatus(L"Minecraft: 未検出");
            UpdateStatus(L"ステータス: 自動モード ON - Minecraftを監視中...");
            EnableControls(true, false, false, true);
        }
    }
    
    // Minecraftを自動検出
    GameProcess foundGame = processDetector->FindMinecraftWindow();
    if (foundGame.hwnd != nullptr) {
        currentGame = foundGame;
        
        std::wstring processName = currentGame.processName;
        UpdateMinecraftStatus(L"Minecraft: 自動検出済み (" + processName + L")");
        UpdateStatus(L"ステータス: Minecraft検出 - 自動ボーダーレス化実行中...");
        
        // 自動的にボーダーレス化を実行
        MakeBorderless();
    }
}

void MinecraftBorderless::MinimizeToTray() {
    if (!trayManager->IsActive()) {
        trayManager->SetupTrayIcon(hMainWindow);
    }
    
    ShowWindow(hMainWindow, SW_HIDE);
    isMinimizedToTray = true;
    UpdateStatus(L"ステータス: システムトレイに最小化しました");
}

void MinecraftBorderless::RestoreFromTray() {
    ShowWindow(hMainWindow, SW_RESTORE);
    SetForegroundWindow(hMainWindow);
    isMinimizedToTray = false;
    
    if (!autoMode) {
        trayManager->CleanupTrayIcon();
    }
}

void MinecraftBorderless::ToggleStartupMode() {
    bool startupMode = SendMessage(hChkStartup, BM_GETCHECK, 0, 0) == BST_CHECKED;
    settingsManager->SetStartupMode(startupMode);
    settingsManager->SaveSettings();
    
    if (startupMode) {
        UpdateStatus(L"ステータス: Windows起動時自動実行 ON");
    } else {
        UpdateStatus(L"ステータス: Windows起動時自動実行 OFF");
    }
}

void MinecraftBorderless::UpdateStatus(const std::wstring& status) {
    SetWindowTextW(hLblStatus, status.c_str());
}

void MinecraftBorderless::UpdateMinecraftStatus(const std::wstring& status) {
    SetWindowTextW(hLblMinecraft, status.c_str());
}

void MinecraftBorderless::UpdateUI() {
    // チェックボックスの状態を設定
    UpdateCheckboxes();
    
    // 初期状態のボタン設定
    EnableControls(true, false, false, false);
    
    // 初期ステータス
    UpdateStatus(L"ステータス: 待機中");
    UpdateMinecraftStatus(L"Minecraft: 未検出");
}

bool MinecraftBorderless::InitializeWindow(HINSTANCE hInstance) {
    // ウィンドウクラスの登録
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = Constants::WINDOW_CLASS;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    
    if (!RegisterClassW(&wc)) {
        return false;
    }
    
    // メインウィンドウの作成
    hMainWindow = CreateWindowExW(
        0,
        Constants::WINDOW_CLASS,
        Constants::APP_NAME,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 420, 380,
        nullptr, nullptr, hInstance, nullptr
    );
    
    return hMainWindow != nullptr;
}

bool MinecraftBorderless::CreateControls(HINSTANCE hInstance) {
    // 検出ボタン
    hBtnDetect = CreateWindowW(
        L"BUTTON", L"Minecraftを検出",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        20, 20, 150, 30,
        hMainWindow, (HMENU)IDC_BTN_DETECT, hInstance, nullptr
    );
    
    // ボーダーレス化ボタン
    hBtnBorderless = CreateWindowW(
        L"BUTTON", L"ボーダーレス化",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | WS_DISABLED,
        20, 60, 150, 30,
        hMainWindow, (HMENU)IDC_BTN_BORDERLESS, hInstance, nullptr
    );
    
    // 復元ボタン
    hBtnRestore = CreateWindowW(
        L"BUTTON", L"元に戻す",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | WS_DISABLED,
        20, 100, 150, 30,
        hMainWindow, (HMENU)IDC_BTN_RESTORE, hInstance, nullptr
    );
    
    // トレイ最小化ボタン
    hBtnMinimize = CreateWindowW(
        L"BUTTON", L"システムトレイに最小化",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | WS_DISABLED,
        20, 140, 180, 30,
        hMainWindow, (HMENU)IDC_BTN_MINIMIZE, hInstance, nullptr
    );
    
    // ステータスラベル
    hLblStatus = CreateWindowW(
        L"STATIC", L"ステータス: 待機中",
        WS_VISIBLE | WS_CHILD,
        20, 185, 350, 20,
        hMainWindow, (HMENU)IDC_LBL_STATUS, hInstance, nullptr
    );
    
    // Minecraftステータスラベル
    hLblMinecraft = CreateWindowW(
        L"STATIC", L"Minecraft: 未検出",
        WS_VISIBLE | WS_CHILD,
        20, 210, 350, 20,
        hMainWindow, (HMENU)IDC_LBL_MINECRAFT, hInstance, nullptr
    );
    
    // 自動モードチェックボックス
    hChkAutoMode = CreateWindowW(
        L"BUTTON", L"自動モード",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        20, 245, 150, 25,
        hMainWindow, (HMENU)IDC_CHK_AUTO_MODE, hInstance, nullptr
    );
    
    // スタートアップチェックボックス
    hChkStartup = CreateWindowW(
        L"BUTTON", L"Windows起動時に自動実行",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        20, 275, 200, 25,
        hMainWindow, (HMENU)IDC_CHK_STARTUP, hInstance, nullptr
    );
    
    return true;
}

void MinecraftBorderless::SetupFonts() {
    HFONT hFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"MS UI Gothic");
    
    if (hFont) {
        SendMessage(hBtnDetect, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hBtnBorderless, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hBtnRestore, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hBtnMinimize, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hLblStatus, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hLblMinecraft, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hChkAutoMode, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hChkStartup, WM_SETFONT, (WPARAM)hFont, TRUE);
    }
}

void MinecraftBorderless::OnCommand(WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    
    switch (LOWORD(wParam)) {
    case IDC_BTN_DETECT:
        DetectMinecraft();
        break;
    case IDC_BTN_BORDERLESS:
        MakeBorderless();
        break;
    case IDC_BTN_RESTORE:
        RestoreWindow();
        break;
    case IDC_BTN_MINIMIZE:
        MinimizeToTray();
        break;
    case IDC_CHK_AUTO_MODE:
        ToggleAutoMode();
        break;
    case IDC_CHK_STARTUP:
        ToggleStartupMode();
        break;
    case IDM_RESTORE:
        RestoreFromTray();
        break;
    case IDM_EXIT:
        PostMessage(hMainWindow, WM_CLOSE, 0, 0);
        break;
    }
}

void MinecraftBorderless::OnTimer(WPARAM wParam) {
    if (wParam == TIMER_AUTO_DETECT) {
        AutoDetectMinecraft();
    }
}

void MinecraftBorderless::OnTrayIcon(WPARAM wParam, LPARAM lParam) {
    trayManager->HandleTrayMessage(hMainWindow, wParam, lParam);
}

void MinecraftBorderless::OnDestroy() {
    PostQuitMessage(0);
}

void MinecraftBorderless::EnableControls(bool detectEnabled, bool borderlessEnabled, bool restoreEnabled, bool minimizeEnabled) {
    EnableWindow(hBtnDetect, detectEnabled ? TRUE : FALSE);
    EnableWindow(hBtnBorderless, borderlessEnabled ? TRUE : FALSE);
    EnableWindow(hBtnRestore, restoreEnabled ? TRUE : FALSE);
    EnableWindow(hBtnMinimize, minimizeEnabled ? TRUE : FALSE);
}

void MinecraftBorderless::UpdateCheckboxes() {
    // 設定からチェックボックスの状態を復元
    SendMessage(hChkAutoMode, BM_SETCHECK, 
        settingsManager->GetAutoMode() ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(hChkStartup, BM_SETCHECK, 
        settingsManager->GetStartupMode() ? BST_CHECKED : BST_UNCHECKED, 0);
} 
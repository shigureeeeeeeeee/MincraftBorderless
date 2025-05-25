#include "MinecraftBorderless.h"

// グローバルインスタンス
MinecraftBorderless* g_pApp = nullptr;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 未使用パラメータの警告を抑制
    UNREFERENCED_PARAMETER(hPrevInstance);
    
    // 共通コントロールを初期化
    InitCommonControls();
    
    // コマンドライン引数をチェック
    bool startupLaunch = false;
    std::string cmdLine = lpCmdLine;
    if (cmdLine.find("-startup") != std::string::npos) {
        startupLaunch = true;
        nCmdShow = SW_HIDE; // スタートアップ時は非表示で開始
    }
    
    // アプリケーションインスタンスを作成
    MinecraftBorderless app;
    g_pApp = &app;
    
    // 初期化
    if (!app.Initialize(hInstance)) {
        MessageBoxW(nullptr, L"アプリケーションの初期化に失敗しました。", L"エラー", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    // スタートアップ起動の場合の処理
    if (startupLaunch) {
        // 自動モードを有効にしてシステムトレイに最小化
        app.ToggleAutoMode();
        app.MinimizeToTray();
    } else {
        // 通常起動の場合はウィンドウを表示
        ShowWindow(app.GetMainWindow(), nCmdShow);
        UpdateWindow(app.GetMainWindow());
    }
    
    // メッセージループ
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // クリーンアップ
    app.Cleanup();
    
    return static_cast<int>(msg.wParam);
} 
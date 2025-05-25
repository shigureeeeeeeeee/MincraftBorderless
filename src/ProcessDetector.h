#pragma once

#include "../include/common.h"

class ProcessDetector {
public:
    ProcessDetector();
    ~ProcessDetector();

    // プロセス検出
    std::vector<DWORD> FindMinecraftProcesses();
    std::vector<GameProcess> FindGameProcesses();
    
    // ウィンドウ検出
    GameProcess FindMinecraftWindow();
    bool IsMinecraftWindow(HWND hwnd);
    
    // プロセス情報取得
    std::wstring GetProcessName(DWORD pid);
    std::wstring GetWindowText(HWND hwnd);
    bool IsProcessRunning(DWORD pid);

private:
    struct EnumWindowsData {
        DWORD targetPID;
        std::vector<HWND> windows;
    };
    
    static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
    bool IsValidMinecraftWindow(HWND hwnd, const std::wstring& title);
}; 
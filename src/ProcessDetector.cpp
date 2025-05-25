#include "ProcessDetector.h"

ProcessDetector::ProcessDetector() {
}

ProcessDetector::~ProcessDetector() {
}

std::vector<DWORD> ProcessDetector::FindMinecraftProcesses() {
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
            if (Utils::IsMinecraftProcess(processName)) {
                processes.push_back(pe32.th32ProcessID);
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }
    
    CloseHandle(hSnapshot);
    return processes;
}

std::vector<GameProcess> ProcessDetector::FindGameProcesses() {
    std::vector<GameProcess> gameProcesses;
    auto pids = FindMinecraftProcesses();
    
    for (DWORD pid : pids) {
        EnumWindowsData data = { pid, {} };
        EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&data));
        
        for (HWND hwnd : data.windows) {
            if (Utils::IsValidGameWindow(hwnd)) {
                GameProcess process;
                process.pid = pid;
                process.hwnd = hwnd;
                process.processName = GetProcessName(pid);
                process.windowTitle = GetWindowText(hwnd);
                gameProcesses.push_back(process);
            }
        }
    }
    
    return gameProcesses;
}

GameProcess ProcessDetector::FindMinecraftWindow() {
    auto processes = FindGameProcesses();
    
    for (const auto& process : processes) {
        if (IsValidMinecraftWindow(process.hwnd, process.windowTitle)) {
            return process;
        }
    }
    
    return {}; // 空のGameProcessを返す
}

bool ProcessDetector::IsMinecraftWindow(HWND hwnd) {
    if (!hwnd || !IsWindow(hwnd) || !IsWindowVisible(hwnd) || GetParent(hwnd) != nullptr) {
        return false;
    }
    
    std::wstring title = GetWindowText(hwnd);
    return IsValidMinecraftWindow(hwnd, title);
}

std::wstring ProcessDetector::GetProcessName(DWORD pid) {
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

std::wstring ProcessDetector::GetWindowText(HWND hwnd) {
    wchar_t title[256];
    GetWindowTextW(hwnd, title, sizeof(title) / sizeof(wchar_t));
    return std::wstring(title);
}

bool ProcessDetector::IsProcessRunning(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (!hProcess) {
        return false;
    }
    
    DWORD exitCode;
    GetExitCodeProcess(hProcess, &exitCode);
    CloseHandle(hProcess);
    
    return exitCode == STILL_ACTIVE;
}

BOOL CALLBACK ProcessDetector::EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);
    
    auto* data = reinterpret_cast<EnumWindowsData*>(lParam);
    
    if (pid == data->targetPID && IsWindowVisible(hwnd) && GetParent(hwnd) == nullptr) {
        data->windows.push_back(hwnd);
    }
    
    return TRUE;
}

bool ProcessDetector::IsValidMinecraftWindow(HWND hwnd, const std::wstring& title) {
    std::wstring lowerTitle = Utils::ToLower(title);
    
    // Minecraftかどうかチェック
    if (lowerTitle.find(L"minecraft") != std::wstring::npos ||
        lowerTitle.find(L"mc") != std::wstring::npos ||
        lowerTitle.length() == 0) { // タイトルなしの場合も許可
        
        // ウィンドウのサイズが妥当かチェック
        RECT rect;
        GetWindowRect(hwnd, &rect);
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;
        
        return width > Constants::MIN_WINDOW_WIDTH && height > Constants::MIN_WINDOW_HEIGHT;
    }
    
    return false;
} 
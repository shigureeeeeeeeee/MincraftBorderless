#include "../include/common.h"
#include <map>

namespace Utils {
    std::wstring ToLower(const std::wstring& str) {
        std::wstring result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::towlower);
        return result;
    }
    
    bool IsMinecraftProcess(const std::wstring& processName) {
        std::wstring lowerName = ToLower(processName);
        
        // より広範囲なMinecraft関連プロセスを検出
        return lowerName.find(L"minecraft") != std::wstring::npos ||
               lowerName.find(L"javaw") != std::wstring::npos ||
               lowerName.find(L"java") != std::wstring::npos ||
               lowerName.find(L"mc") != std::wstring::npos;
    }
    
    bool IsValidGameWindow(HWND hwnd) {
        if (!hwnd || !IsWindow(hwnd) || !IsWindowVisible(hwnd) || GetParent(hwnd) != nullptr) {
            return false;
        }
        
        // ウィンドウのサイズが妥当かチェック
        RECT rect;
        GetWindowRect(hwnd, &rect);
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;
        
        return width > Constants::MIN_WINDOW_WIDTH && height > Constants::MIN_WINDOW_HEIGHT;
    }
} 
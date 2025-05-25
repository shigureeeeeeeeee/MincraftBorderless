#include "SettingsManager.h"
#include <fstream>
#include <sstream>

SettingsManager::SettingsManager() {
    settingsFilePath = GetSettingsFilePath();
}

SettingsManager::~SettingsManager() {
}

bool SettingsManager::LoadSettings() {
    // レジストリからスタートアップ設定を読み込み
    settings.startupMode = IsStartupRegistered();
    
    // 将来的にはINIファイルやJSONファイルから他の設定を読み込み可能
    // 現在は基本的な設定のみ
    
    return true;
}

bool SettingsManager::SaveSettings() {
    // スタートアップ設定をレジストリに保存
    SetStartupRegistration(settings.startupMode);
    
    // 将来的には他の設定もファイルに保存
    
    return true;
}

bool SettingsManager::SetStartupRegistration(bool enable) {
    const std::wstring keyPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
    const std::wstring valueName = L"MinecraftBorderless";
    
    if (enable) {
        std::wstring exePath = GetExecutablePath();
        std::wstring commandLine = L"\"" + exePath + L"\" -startup";
        return WriteToRegistry(keyPath, valueName, commandLine);
    } else {
        return DeleteFromRegistry(keyPath, valueName);
    }
}

bool SettingsManager::IsStartupRegistered() {
    const std::wstring keyPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
    const std::wstring valueName = L"MinecraftBorderless";
    
    std::wstring value;
    return ReadFromRegistry(keyPath, valueName, value);
}

std::wstring SettingsManager::GetSettingsFilePath() {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    
    std::wstring exePath = path;
    size_t lastSlash = exePath.find_last_of(L"\\");
    if (lastSlash != std::wstring::npos) {
        return exePath.substr(0, lastSlash + 1) + L"settings.ini";
    }
    
    return L"settings.ini";
}

std::wstring SettingsManager::GetExecutablePath() {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    return std::wstring(path);
}

bool SettingsManager::WriteToRegistry(const std::wstring& keyPath, const std::wstring& valueName, const std::wstring& value) {
    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_WRITE, &hKey);
    
    if (result != ERROR_SUCCESS) {
        return false;
    }
    
    result = RegSetValueExW(hKey, valueName.c_str(), 0, REG_SZ, 
        (const BYTE*)value.c_str(), 
        (value.length() + 1) * sizeof(wchar_t));
    
    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
}

bool SettingsManager::ReadFromRegistry(const std::wstring& keyPath, const std::wstring& valueName, std::wstring& value) {
    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_READ, &hKey);
    
    if (result != ERROR_SUCCESS) {
        return false;
    }
    
    DWORD dataSize = 0;
    result = RegQueryValueExW(hKey, valueName.c_str(), nullptr, nullptr, nullptr, &dataSize);
    
    if (result == ERROR_SUCCESS && dataSize > 0) {
        std::vector<wchar_t> buffer(dataSize / sizeof(wchar_t));
        result = RegQueryValueExW(hKey, valueName.c_str(), nullptr, nullptr, 
            (BYTE*)buffer.data(), &dataSize);
        
        if (result == ERROR_SUCCESS) {
            value = buffer.data();
        }
    }
    
    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
}

bool SettingsManager::DeleteFromRegistry(const std::wstring& keyPath, const std::wstring& valueName) {
    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_WRITE, &hKey);
    
    if (result != ERROR_SUCCESS) {
        return false;
    }
    
    result = RegDeleteValueW(hKey, valueName.c_str());
    RegCloseKey(hKey);
    
    return result == ERROR_SUCCESS || result == ERROR_FILE_NOT_FOUND;
} 
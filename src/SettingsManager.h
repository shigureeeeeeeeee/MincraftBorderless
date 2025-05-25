#pragma once

#include "../include/common.h"

struct AppSettings {
    bool autoMode = false;
    bool startupMode = false;
    bool enableNotifications = true;
    int autoDetectInterval = Constants::AUTO_DETECT_INTERVAL;
    std::wstring lastMinecraftPath;
};

class SettingsManager {
public:
    SettingsManager();
    ~SettingsManager();

    // 設定の読み込み・保存
    bool LoadSettings();
    bool SaveSettings();
    
    // スタートアップ登録
    bool SetStartupRegistration(bool enable);
    bool IsStartupRegistered();
    
    // 設定アクセス
    const AppSettings& GetSettings() const { return settings; }
    void SetSettings(const AppSettings& newSettings) { settings = newSettings; }
    
    // 個別設定アクセス
    bool GetAutoMode() const { return settings.autoMode; }
    void SetAutoMode(bool value) { settings.autoMode = value; }
    
    bool GetStartupMode() const { return settings.startupMode; }
    void SetStartupMode(bool value) { settings.startupMode = value; }
    
    bool GetEnableNotifications() const { return settings.enableNotifications; }
    void SetEnableNotifications(bool value) { settings.enableNotifications = value; }
    
    int GetAutoDetectInterval() const { return settings.autoDetectInterval; }
    void SetAutoDetectInterval(int value) { settings.autoDetectInterval = value; }

private:
    AppSettings settings;
    std::wstring settingsFilePath;
    
    std::wstring GetSettingsFilePath();
    std::wstring GetExecutablePath();
    bool WriteToRegistry(const std::wstring& keyPath, const std::wstring& valueName, const std::wstring& value);
    bool ReadFromRegistry(const std::wstring& keyPath, const std::wstring& valueName, std::wstring& value);
    bool DeleteFromRegistry(const std::wstring& keyPath, const std::wstring& valueName);
}; 
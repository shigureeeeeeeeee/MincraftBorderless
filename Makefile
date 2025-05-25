# Minecraft Borderless Tool Makefile

# コンパイラとフラグ
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -mwindows
INCLUDES = -Iinclude
LIBS = -lcomctl32 -lshell32 -lshlwapi

# ディレクトリ
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

# ソースファイル
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# ターゲット
TARGET = $(BUILD_DIR)/MinecraftBorderless.exe
LEGACY_TARGET = MinecraftBorderless.exe

# デフォルトターゲット
all: $(TARGET)

# メインターゲット
$(TARGET): $(OBJECTS) | $(BUILD_DIR)
	$(CXX) $(OBJECTS) -o $@ $(LIBS) $(CXXFLAGS)
	@echo "ビルド完了: $@"

# オブジェクトファイル
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# ディレクトリ作成
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# レガシーファイルのクリーンアップ
clean-legacy:
	@if exist $(LEGACY_TARGET) del $(LEGACY_TARGET)
	@echo "レガシーファイルをクリーンアップしました"

# クリーンアップ
clean:
	@if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
	@echo "ビルドファイルをクリーンアップしました"

# 完全クリーンアップ
clean-all: clean clean-legacy

# リビルド
rebuild: clean all

# インストール（現在のディレクトリにコピー）
install: $(TARGET)
	copy $(BUILD_DIR)\MinecraftBorderless.exe .
	@echo "実行ファイルをインストールしました"

# ヘルプ
help:
	@echo "利用可能なターゲット:"
	@echo "  all          - プロジェクトをビルド"
	@echo "  clean        - ビルドファイルを削除"
	@echo "  clean-legacy - レガシーファイルを削除"
	@echo "  clean-all    - 全てのファイルを削除"
	@echo "  rebuild      - クリーンビルド"
	@echo "  install      - 実行ファイルをインストール"
	@echo "  help         - このヘルプを表示"

.PHONY: all clean clean-legacy clean-all rebuild install help 
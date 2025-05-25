# Minecraft ボーダーレス化ツール

Minecraftをボーダーレス（フルスクリーン）モードで実行するためのWindows用ツールです。

## 🚀 機能

- **自動検出**: Minecraftプロセスの自動検出
- **ボーダーレス化**: ワンクリックでボーダーレス表示
- **自動モード**: Minecraft起動時の自動ボーダーレス化
- **システムトレイ**: バックグラウンド実行とトレイアイコン
- **スタートアップ**: Windows起動時の自動実行
- **復元機能**: 元のウィンドウ状態への復元

## 📁 プロジェクト構造

```
MinecraftBorderless/
├── src/                          # ソースコード
│   ├── main.cpp                  # エントリーポイント
│   ├── MinecraftBorderless.h/.cpp # メインクラス
│   ├── ProcessDetector.h/.cpp    # プロセス検出
│   ├── WindowManager.h/.cpp      # ウィンドウ操作
│   ├── TrayManager.h/.cpp        # システムトレイ管理
│   ├── SettingsManager.h/.cpp    # 設定管理
│   └── Utils.cpp                 # ユーティリティ関数
├── include/                      # ヘッダーファイル
│   ├── common.h                  # 共通定義
│   └── resource.h                # リソースID
├── build/                        # ビルド出力
├── build.ps1                     # PowerShellビルドスクリプト
├── Makefile                      # Makefileビルドスクリプト
└── MinecraftBorderless.exe       # 実行ファイル
```

## 🔧 ビルド方法

### PowerShell使用（推奨）

```powershell
# 通常ビルド
.\build.ps1

# クリーンビルド
.\build.ps1 -Rebuild

# 実行ファイルをインストール
.\build.ps1 -Install

# ヘルプを表示
.\build.ps1 -Help
```

### Make使用

```bash
# 通常ビルド
make

# クリーンビルド
make rebuild

# 実行ファイルをインストール
make install
```

## 📋 必要な環境

- **OS**: Windows 10/11
- **コンパイラ**: MinGW-w64 (g++)
- **C++標準**: C++17以上
- **依存ライブラリ**: 
  - comctl32.lib
  - shell32.lib
  - shlwapi.lib

## 🎮 使用方法

1. **基本的な使用**
   - `MinecraftBorderless.exe`を実行
   - 「Minecraftを検出」ボタンをクリック
   - 「ボーダーレス化」ボタンをクリック

2. **自動モード**
   - 「自動モード」チェックボックスを有効化
   - Minecraft起動時に自動でボーダーレス化

3. **スタートアップ設定**
   - 「Windows起動時に自動実行」チェックボックスを有効化
   - システム起動時に自動でツールが開始

## 🏗️ アーキテクチャ

### クラス設計

- **MinecraftBorderless**: メインアプリケーションクラス
- **ProcessDetector**: プロセス検出とウィンドウ特定
- **WindowManager**: ウィンドウ操作とモニター管理
- **TrayManager**: システムトレイアイコン管理
- **SettingsManager**: 設定の保存・読み込み

### 設計原則

- **単一責任原則**: 各クラスが明確な責任を持つ
- **依存性注入**: 管理クラスをunique_ptrで管理
- **カプセル化**: プライベートメンバーの適切な分離

## 🔄 リファクタリング履歴

このプロジェクトは保守性と拡張性を向上させるために大幅にリファクタリングされました：

### Before (モノリシック)
- 単一ファイル（768行）
- 全機能が一つのクラスに集約
- 保守が困難

### After (モジュラー)
- 複数ファイルに分割
- 責任分離されたクラス設計
- 拡張しやすい構造

## 🛠️ 今後の拡張予定

- マルチモニター対応
- ホットキー機能
- プロファイル機能
- 詳細設定ダイアログ
- 通知機能の強化

## 📝 ライセンス

このプロジェクトはMITライセンスの下で公開されています。

## 🤝 貢献

プルリクエストやイシューの報告を歓迎します！

## システム要件

- Windows 10/11 (x86/x64)
- Java版Minecraft

## 注意事項

- 管理者権限は不要です
- Minecraftが実行中の状態で使用してください
- Java版Minecraftのみ対応（Bedrock版は非対応）
- ウィンドウのタイトルまたはプロセス名に「minecraft」または「javaw」が含まれるプロセスを対象とします

## トラブルシューティング

### Minecraftが検出されない場合
- Minecraftが確実に起動しているか確認してください
- Minecraftのウィンドウが最小化されていないか確認してください
- 他のJavaアプリケーションが干渉していないか確認してください

### ボーダーレス化できない場合
- 管理者権限でツールを実行してみてください
- Minecraftを再起動してから再試行してください

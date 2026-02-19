# USD Asset Resolver Plugin

OpenUSD の `asset:` URI スキームを処理する ArResolver プラグイン。

## 概要

`asset:chair_0` や `asset:chair_0?v=10` のようなアセットパスを実ファイルパスに解決します。

| 入力 | 解決先 |
|------|--------|
| `asset:chair_0` | `{USD_ASSET_ROOT}/chair_0/{versions.json の latest}/chair_0.usd` |
| `asset:chair_0?v=10` | `{USD_ASSET_ROOT}/chair_0/v10/chair_0.usd` |

バージョンを省略した場合は、アセットフォルダ内の `versions.json` の `"latest"` キーから解決されます。

## 必要環境

- Houdini 21.0（USD SDK 同梱）
- CMake 3.19 以上
- Visual Studio 2022（Windows）

## ビルド

`HFS` 環境変数が未設定の場合、Windows では `C:\Program Files\Side Effects Software\Houdini 21.*` を自動検出します。

### cmd

```bat
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
cmake --install . --config Release
```

### PowerShell

```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
cmake --install . --config Release
```

### CMake オプション

| オプション | デフォルト | 説明 |
|-----------|-----------|------|
| `HFS` | 自動検出 | Houdini インストールパス（環境変数または `-DHFS=<path>`） |
| `PYTHON_VERSION` | `3.11` | 使用する Python バージョン（例: `3.11`, `3.10`, `3.9`） |
| `CMAKE_INSTALL_PREFIX` | `dist/` | インストール先ディレクトリ |

Python バージョンを変更する場合:

```powershell
cmake .. -G "Visual Studio 17 2022" -A x64 -D"PYTHON_VERSION=3.9"
```

### ビルド成果物

- `dist/lib/assetPathResolver.dll` - プラグイン本体
- `dist/resources/plugInfo.json` - プラグイン登録情報

## テスト

### cmd

```bat
REM 1. 環境変数を設定
call setup_env.bat

REM 2. テスト用アセットを作成
"%HFS%\bin\hython.exe" tests\create_test_assets.py

REM 3. テスト実行
"%HFS%\bin\hython.exe" tests\test_resolve.py
```

### PowerShell

```powershell
# 1. 環境変数を設定（ドットソースで現在のセッションに反映）
. .\setup_env.ps1

# 2. テスト用アセットを作成
& "$env:HFS\bin\hython.exe" tests\create_test_assets.py

# 3. テスト実行
& "$env:HFS\bin\hython.exe" tests\test_resolve.py
```

## 環境変数

| 変数 | 説明 |
|------|------|
| `USD_ASSET_ROOT` | アセットのルートディレクトリ |
| `PXR_PLUGINPATH_NAME` | `dist/resources` を追加 |
| `PATH` | `dist/lib` を追加（Windows） |

## アセットディレクトリ構成

```
{USD_ASSET_ROOT}/
└── {asset_name}/
    ├── versions.json       ← latest バージョンの定義
    ├── v1/
    │   └── {asset_name}.usd
    └── v10/
        └── {asset_name}.usd
```

`versions.json` の例:
```json
{
  "name": "chair_0",
  "latest": "v10"
}
```

`asset:chair_0`（バージョン未指定）は `versions.json` の `"latest"` を参照し、該当バージョンのファイルに解決されます。

## AssetInfoAPI（Codeless Schema）

Prim にアセットメタデータを付与する Single-Apply API Schema です。C++ コード不要で、`generatedSchema.usda` と `plugInfo.json` のみで定義されています。

### 属性

| 属性 | 型 | デフォルト | 説明 |
|------|------|-----------|------|
| `assetInfo:version` | string | `""` | バージョン文字列（例: `"v10"`） |
| `assetInfo:author` | string | `""` | 作者 |
| `assetInfo:status` | token | `"draft"` | ステータス（`draft` / `review` / `published` / `deprecated`） |
| `assetInfo:description` | string | `""` | 説明 |

### 使用例

```python
from pxr import Usd

stage = Usd.Stage.Open("asset:chair_0")
prim = stage.GetPrimAtPath("/chair_0")

prim.ApplyAPI("AssetInfoAPI")
prim.GetAttribute("assetInfo:version").Set("v10")
prim.GetAttribute("assetInfo:author").Set("hideki")
prim.GetAttribute("assetInfo:status").Set("published")
```

### テスト

```powershell
. .\setup_env.ps1
& "$env:HFS\bin\hython.exe" tests\test_schema.py
```

## デバッグ

### cmd

```bat
set TF_DEBUG=ASSETPATHRESOLVER_RESOLVER
"%HFS%\bin\hython.exe" tests\test_resolve.py
```

### PowerShell

```powershell
$env:TF_DEBUG = "ASSETPATHRESOLVER_RESOLVER"
& "$env:HFS\bin\hython.exe" tests\test_resolve.py
```

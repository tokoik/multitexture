# multitexture

OpenGL のマルチテクスチャ機能を使ったサンプルプログラム

## 1. プログラムの説明

### 1.1 マルチテクスチャとは

マルチテクスチャ (Multitexture) とは、1 つのポリゴンに複数のテクスチャを同時に貼り合わせる機能です。OpenGL 1.3 以降では標準仕様として組み込まれていますが、それ以前の環境では `GL_ARB_multitexture` 拡張として提供されていました。本プログラムは `GL_ARB_multitexture` 拡張の API (`glActiveTextureARB`、`glMultiTexCoord2fARB`) を使って実装しています。

### 1.2 テクスチャユニット

マルチテクスチャは **テクスチャユニット** という複数のスロットを持ちます。各ユニットに独立したテクスチャを割り当て、それぞれ異なる合成方法でポリゴンに適用できます。

| ユニット | 定数 | 本プログラムでの役割 |
| --- | --- | --- |
| 0 | `GL_TEXTURE0_ARB` | ベーステクスチャ (`texture0.raw`) を `GL_REPLACE` で貼る |
| 1 | `GL_TEXTURE1_ARB` | 追加テクスチャ (`texture1.raw`) を `GL_MODULATE` で乗算合成する |

### 1.3 主要な API

| 関数 | 説明 |
| --- | --- |
| `glActiveTextureARB(unit)` | 操作対象のテクスチャユニットを切り替える |
| `glMultiTexCoord2fARB(unit, s, t)` | 頂点ごとに各ユニット用のテクスチャ座標を指定する |
| `glBindTexture(GL_TEXTURE_2D, name)` | アクティブなユニットにテクスチャをバインドする |
| `glTexEnvi(..., GL_TEXTURE_ENV_MODE, mode)` | テクスチャの合成方法を指定する |

### 1.4 テクスチャの合成順序

各テクスチャユニットは **番号の小さい順** に適用されます。

```text
最終色 = texName1 × ( texName0 の置換色 )
         ^^^^^^^^   ^^^^^^^^^^^^^^^^^^^^
        MODULATE      REPLACE (ベース)
```

ユニット 0 で `GL_REPLACE` によりポリゴン色をテクスチャ 0 の色に置き換え、ユニット 1 で `GL_MODULATE` によりテクスチャ 1 の色を乗算合成します。

### 1.5 テクスチャファイル

| ファイル | フォーマット | サイズ |
| --- | --- | --- |
| `texture0.raw` | 生 RGB (各チャネル 8 bit、ヘッダなし) | 256 × 256 × 3 bytes |
| `texture1.raw` | 生 RGB (各チャネル 8 bit、ヘッダなし) | 256 × 256 × 3 bytes |

実行ファイルと同じディレクトリに配置してください。

### 1.6 Windows での関数ポインタの取得

Windows (MSVC) 環境では `GL_ARB_multitexture` の関数がリンク時に解決されないため、`wglGetProcAddress` を使って実行時に関数ポインタを取得しています。

```c
glActiveTextureARB =
    (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");
glMultiTexCoord2fARB =
    (PFNGLMULTITEXCOORD2FARBPROC)wglGetProcAddress("glMultiTexCoord2fARB");
```

macOS・Linux ではこの処理は不要で、関数を直接呼び出せます。

### 1.7 操作方法

| キー | 動作 |
| --- | --- |
| `q` / `Q` / `Esc` | プログラムを終了する |

## 2. ビルド方法

### 2.1 共通の前提条件

CMake 3.18 以降が必要です。
Windows 向けの `glext.h` および `freeglut` は CMake の設定時に自動ダウンロードされます。

### 2.2 Windows (Visual Studio 2022)

#### 2.2.1 必要なもの

- Visual Studio 2022 (C++ ワークロードをインストール済み)
- CMake 3.18 以降 ([cmake.org](https://cmake.org/) または Visual Studio インストーラーで導入)

#### 2.2.2 手順

```bat
:: ソースディレクトリへ移動
cd multitexture

:: ビルドディレクトリを作成して Visual Studio 2022 用プロジェクトを生成
cmake -S . -B build -G "Visual Studio 17 2022" -A x64

:: コマンドラインでビルドする場合
cmake --build build --config Release
```

または CMake が生成した `build\multitexture.sln` を Visual Studio 2022 で開き、構成を **Release** にしてビルドしてください。

#### 2.2.3 実行

ビルド成功後、`build\Release\multitexture.exe` が生成されます。`texture0.raw` と `texture1.raw` を実行ファイルと同じディレクトリに置いて実行してください。

### 2.3 macOS (Xcode)

#### 2.3.1 必要なもの

- Xcode (App Store からインストール)
- Xcode コマンドラインツール (`xcode-select --install`)
- CMake 3.18 以降 (`brew install cmake` または [cmake.org](https://cmake.org/))

#### 2.3.2 手順

```sh
# ソースディレクトリへ移動
cd multitexture

# Xcode プロジェクトを生成
cmake -S . -B build -G Xcode

# コマンドラインでビルドする場合
cmake --build build --config Release
```

または `build/multitexture.xcodeproj` を Xcode で開いてビルドしてください。

#### 2.3.3 実行

`build/Release/multitexture` が生成されます。`texture0.raw` と `texture1.raw` を実行ファイルと同じディレクトリに置いて実行してください。

### 2.4 Ubuntu Linux (CMake + pkg-config)

#### 2.4.1 必要なもの

```sh
sudo apt update
sudo apt install build-essential cmake freeglut3-dev
```

| パッケージ | 説明 |
| --- | --- |
| `build-essential` | gcc, make などのビルドツール |
| `cmake` | CMake ビルドシステム |
| `freeglut3-dev` | freeglut の開発ファイル (ヘッダ・ライブラリ) |

#### 2.4.2 pkg-config について

このプロジェクトの CMakeLists.txt は Linux 環境では `pkg-config` を使って freeglut の情報（ヘッダファイルのパス、リンクするライブラリなど）を自動取得します。

```sh
# 利用可能な freeglut パッケージの情報を確認（参考）
pkg-config --cflags --libs freeglut
# 出力例: -I/usr/include/GL -L/usr/lib/x86_64-linux-gnu -lfreeglut -lGL -lGLU -lm
```

#### 2.4.3 ビルド方法

```sh
# ソースディレクトリへ移動
cd multitexture

# ビルドディレクトリを作成して CMake でプロジェクトを設定
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# コマンドラインでビルド
cmake --build build
```

#### 2.4.4 実行

```sh
# ビルド成功後、実行ファイルが生成されます
build/multitexture

# 別ターミナルから実行する場合は texture*.raw ファイルを同じディレクトリに配置
cp texture0.raw texture1.raw build/
./build/multitexture
```

> **Note:** `cmake --build build` は内部的に `make` を呼び出しており、その際に pkg-config で取得した CFLAGS・LDFLAGS が自動的に使用されます。

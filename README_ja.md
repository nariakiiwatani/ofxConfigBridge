# ofxConfigBridge

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![openFrameworks](https://img.shields.io/badge/openFrameworks-0.12.0+-brightgreen.svg)](https://openframeworks.cc/)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)

**ofxConfigBridge** は、openFrameworks 用のアドオンで、
**YAML、JSON、TOML** など複数の設定ファイル形式を
統一的に読み書き・変換するためのライブラリです。
将来的に INI や XML などの拡張も視野に入れています。

---

## ✨ 特徴

- 🧩 **統一 API** によるファイル入出力とフォーマット変換
- 📄 **マルチフォーマット対応**:
  - YAML (`yaml-cpp`)
  - JSON (`nlohmann::json`)
  - Ordered JSON (`nlohmann::ordered_json`)
  - TOML (`toml11` - 順序付き・非順序付き)
- 🔄 **シームレスな変換** - サポートされている任意のフォーマット間で変換可能
- 📂 **自動フォーマット検出** - ファイル拡張子による自動判定
- 🧱 **拡張可能な構造** - アダプターパターンによる設計
- 🛠️ **豊富な API** - ファイル入出力、テキスト解析・出力、変換機能

---

## 🛠️ インストール

```bash
cd openFrameworks/addons
git clone https://github.com/nariakiiwatani/ofxConfigBridge.git
```

---

## 📖 API リファレンス

### 基本的なファイル操作

```cpp
#include "ofxConfigBridge.h"
using namespace ofx::configbridge;

// 設定ファイルの読み込み
auto yaml = loadFile<YAML::Node>("config.yaml");
auto json = loadFile<nlohmann::ordered_json>("config.json");
auto toml = loadFile<toml::ordered_value>("config.toml");

// 設定ファイルの保存
saveFile("output.yaml", yaml);
saveFile("output.json", json);
saveFile("output.toml", toml);
```

### フォーマット変換

```cpp
// サポートされている任意のフォーマット間で変換
auto yaml = loadFile<YAML::Node>("config.yaml");
auto json = convert<nlohmann::ordered_json>(yaml);
auto toml = convert<toml::ordered_value>(json);

// 変換の連鎖
auto result = convert<YAML::Node>(
    convert<nlohmann::json>(
        loadFile<toml::ordered_value>("config.toml")
    )
);
```

### テキスト処理

```cpp
// 文字列からの解析
std::string yamlText = "key: value\nlist: [1, 2, 3]";
YAML::Node parsed;
parseText(yamlText, parsed);

// 文字列への出力
std::string output;
dumpText(parsed, output);
```

### サポートされる型

| フォーマット | 型 | 説明 |
|-------------|----|----- |
| JSON | [`nlohmann::json`](src/ofxConfigBridgeAdapterJson.hpp:14) | 標準的な非順序 JSON |
| JSON | [`nlohmann::ordered_json`](src/ofxConfigBridgeAdapterJson.hpp:15) | 順序保持 JSON |
| YAML | [`YAML::Node`](src/ofxConfigBridgeAdapterYamlCpp.hpp:9) | YAML ドキュメント |
| TOML | [`toml::ordered_value`](src/ofxConfigBridgeAdapterToml.hpp:11) | 順序保持 TOML |
| TOML | [`toml::value`](src/ofxConfigBridgeAdapterToml.hpp:13) | 標準 TOML |

---

## 🔧 設定オプション

```cpp
Options opt;
opt.auto_number_conversion = true;   // 数値文字列の自動変換
opt.auto_boolean_conversion = true;  // ブール文字列の自動変換
opt.float_precision = 6;             // 浮動小数点の出力精度

auto result = loadFile<YAML::Node>("config.yaml", opt);
```

---

## 📦 依存関係

このアドオンには以下のライブラリが含まれています：

- **[yaml-cpp](libs/yaml-cpp/)** (MIT) - YAML の解析と出力
- **[toml11](libs/toml11/)** (MIT) - TOML の解析とシリアライゼーション
- **nlohmann/json** (openFrameworks に含まれる) - JSON 処理
- **openFrameworks 0.12.0 以上**

---

## 🎯 使用例

完全な動作例は [`example/src/ofApp.cpp`](example/src/ofApp.cpp) を参照してください：

```cpp
void ofApp::setup() {
    using namespace ofx::configbridge;
    
    // YAML を読み込んで他のフォーマットに変換
    auto yaml = loadFile<YAML::Node>("config.yaml");
    auto json = convert<nlohmann::ordered_json>(yaml);
    auto toml = convert<toml::ordered_value>(yaml);
    
    // 変換されたファイルを保存
    saveFile("yaml_to_json.json", json);
    saveFile("yaml_to_toml.toml", toml);
    
    // クロスフォーマット変換もシームレスに動作
    auto backToYaml = convert<YAML::Node>(toml);
    saveFile("full_circle.yaml", backToYaml);
}
```

---

## ⚖️ ライセンス

このプロジェクトは MIT ライセンスで公開されています。
全文は [`LICENSE`](LICENSE) を参照してください。

**同梱ライブラリ:**
- **yaml-cpp** (MIT, © 2008 Jesse Beder)
- **toml11** (MIT, © 2017 ToruNiina)

# ofxConfigBridge

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![openFrameworks](https://img.shields.io/badge/openFrameworks-0.12.0+-brightgreen.svg)](https://openframeworks.cc/)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)

**ofxConfigBridge** は、openFrameworks 用のアドオンで、  
**YAML / JSON / Ordered JSON** など複数の設定ファイル形式を
統一的に読み書き・変換するためのライブラリです。  
将来的に TOML / INI / XML などの拡張も視野に入れています。

---

## ✨ 特徴

- 🧩 設定ファイルフォーマット間の統一 API
- 🧾 対応形式
  - YAML (`yaml-cpp`)
  - JSON (`nlohmann::json`)
  - Ordered JSON (`nlohmann::ordered_json`)
- 🔄 YAML ⇄ JSON の双方向変換
- 🧱 拡張可能なアダプタ／コンバータ構造

---

## 🛠️ インストール

```bash
cd openFrameworks/addons
git clone https://github.com/nariakiiwatani/ofxConfigBridge.git
```

---

## 📖 使用例

```cpp
#include "ofxConfigBridge.h"

void ofApp::setup() {
    using namespace ofx::configbridge;

    auto yaml = ofxConfigBridge::loadFile<YAML::Node>("config.yaml");
    auto json = ofxConfigBridge::convert<nlohmann::ordered_json>(yaml);

    json["window"]["width"] = 1280;
    ofxConfigBridge::saveFile("config.json", json);
}

```

---

## 📦 依存関係

- [yaml-cpp](https://github.com/jbeder/yaml-cpp) (MIT)
- openFrameworks 0.12.0 以上

---

## ⚖️ ライセンス

このプロジェクトは MIT ライセンスで公開されています。  
全文は [`LICENSE`](LICENSE) を参照してください。

同梱:
- **yaml-cpp** (MIT, © 2008 Jesse Beder)

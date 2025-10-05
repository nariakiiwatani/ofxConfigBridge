# ofxConfigBridge

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![openFrameworks](https://img.shields.io/badge/openFrameworks-0.12.0+-brightgreen.svg)](https://openframeworks.cc/)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)

**ofxConfigBridge** is an openFrameworks addon that provides a unified interface for
reading, writing, and converting between multiple configuration file formats —  
including **YAML**, **JSON**, and **ordered JSON** — with extensible support for
future formats such as TOML, INI, and XML.

---

## ✨ Features

- 🧩 Unified API for file I/O and format conversion
- 🧾 Supports:
  - YAML (`yaml-cpp`)
  - JSON (`nlohmann::json`)
  - Ordered JSON (`nlohmann::ordered_json`)
- 🔄 Format-to-format converters (e.g. YAML ⇄ JSON)
- 🧱 Extensible adapter/bridge architecture

---

## 🛠️ Installation

```bash
cd openFrameworks/addons
git clone https://github.com/nariakiiwatani/ofxConfigBridge.git
```

---

## 📖 Usage Example

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

## 📦 Dependencies

- [yaml-cpp](https://github.com/jbeder/yaml-cpp) (MIT)
- openFrameworks 0.12.0+

---

## ⚖️ License

This project is licensed under the MIT License.  
See [`LICENSE`](LICENSE) for full text.

Includes:
- **yaml-cpp** (MIT, © 2008 Jesse Beder)

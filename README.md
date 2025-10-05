# ofxConfigBridge

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![openFrameworks](https://img.shields.io/badge/openFrameworks-0.12.0+-brightgreen.svg)](https://openframeworks.cc/)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)

**ofxConfigBridge** is an openFrameworks addon that provides a unified interface for
reading, writing, and converting between multiple configuration file formats —
including **YAML**, **JSON**, and **TOML** — with extensible support for
future formats such as INI and XML.

---

## ✨ Features

- 🧩 **Unified API** for file I/O and format conversion
- 📄 **Multiple Format Support**:
  - YAML (`yaml-cpp`)
  - JSON (`nlohmann::json`)
  - Ordered JSON (`nlohmann::ordered_json`)
  - TOML (`toml11` - ordered & unordered)
- 🔄 **Seamless Conversion** between any supported formats
- 📂 **Auto-format Detection** based on file extensions
- 🧱 **Extensible Architecture** with adapter pattern
- 🛠️ **Rich API** including file I/O, text parsing/dumping, and conversion

---

## 🛠️ Installation

```bash
cd openFrameworks/addons
git clone https://github.com/nariakiiwatani/ofxConfigBridge.git
```

---

## 📖 API Reference

### Basic File Operations

```cpp
#include "ofxConfigBridge.h"
using namespace ofx::configbridge;

// Load configuration files
auto yaml = loadFile<YAML::Node>("config.yaml");
auto json = loadFile<nlohmann::ordered_json>("config.json");
auto toml = loadFile<toml::ordered_value>("config.toml");

// Save configuration files
saveFile("output.yaml", yaml);
saveFile("output.json", json);
saveFile("output.toml", toml);
```

### Format Conversion

```cpp
// Convert between any supported formats
auto yaml = loadFile<YAML::Node>("config.yaml");
auto json = convert<nlohmann::ordered_json>(yaml);
auto toml = convert<toml::ordered_value>(json);

// Chain conversions
auto result = convert<YAML::Node>(
    convert<nlohmann::json>(
        loadFile<toml::ordered_value>("config.toml")
    )
);
```

### Text Processing

```cpp
// Parse from string
std::string yamlText = "key: value\nlist: [1, 2, 3]";
YAML::Node parsed;
parseText(yamlText, parsed);

// Dump to string
std::string output;
dumpText(parsed, output);
```

### Supported Types

| Format | Type | Description |
|--------|------|-------------|
| JSON | [`nlohmann::json`](src/ofxConfigBridgeAdapterJson.hpp:14) | Standard unordered JSON |
| JSON | [`nlohmann::ordered_json`](src/ofxConfigBridgeAdapterJson.hpp:15) | Order-preserving JSON |
| YAML | [`YAML::Node`](src/ofxConfigBridgeAdapterYamlCpp.hpp:9) | YAML documents |
| TOML | [`toml::ordered_value`](src/ofxConfigBridgeAdapterToml.hpp:11) | Order-preserving TOML |
| TOML | [`toml::value`](src/ofxConfigBridgeAdapterToml.hpp:13) | Standard TOML |

---

## 🔧 Configuration Options

```cpp
// Options are used for conversion and export operations
Options opt;
opt.float_precision = 6;  // Control floating-point output precision

// Convert with options
auto json = convert<nlohmann::json>(yaml, opt);

// Export to text with options
std::string output;
dumpText(data, output, opt);

// Save file with options
saveFile("output.json", data, opt);
```

---

## 📦 Dependencies

This addon includes the following libraries:

- **[yaml-cpp](libs/yaml-cpp/)** (MIT) - YAML parsing and emission
- **[toml11](libs/toml11/)** (MIT) - TOML parsing and serialization
- **nlohmann/json** (included with openFrameworks) - JSON processing
- **openFrameworks 0.12.0+**

---

## 🎯 Example Usage

See the complete working example in [`example/src/ofApp.cpp`](example/src/ofApp.cpp):

```cpp
void ofApp::setup() {
    using namespace ofx::configbridge;
    
    // Load YAML and convert to other formats
    auto yaml = loadFile<YAML::Node>("config.yaml");
    auto json = convert<nlohmann::ordered_json>(yaml);
    auto toml = convert<toml::ordered_value>(yaml);
    
    // Save converted files
    saveFile("yaml_to_json.json", json);
    saveFile("yaml_to_toml.toml", toml);
    
    // Cross-format conversions work seamlessly
    auto backToYaml = convert<YAML::Node>(toml);
    saveFile("full_circle.yaml", backToYaml);
}
```

---

## ⚖️ License

This project is licensed under the MIT License.
See [`LICENSE`](LICENSE) for full text.

**Included Libraries:**
- **yaml-cpp** (MIT, © 2008 Jesse Beder)
- **toml11** (MIT, © 2017 ToruNiina)

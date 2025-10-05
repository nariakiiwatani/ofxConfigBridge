#pragma once
#include <variant>
#include <nlohmann/json.hpp>
#include "yaml.h"
#include "toml.hpp"

namespace ofx { namespace configbridge {

struct Document {
    enum class Type { None, Json, Yaml, Toml };

    using JsonDom = nlohmann::ordered_json;
    using YamlDom = YAML::Node;
    using TomlDom = toml::ordered_value;

    using Storage = std::variant<std::monostate, JsonDom, YamlDom, TomlDom>;

    Type    type = Type::None;
    Storage dom;

    bool empty() const { return type == Type::None; }

    template<typename T>       T& as()       { return std::get<T>(dom); }
    template<typename T> const T& as() const { return std::get<T>(dom); }

    JsonDom& ojson(){ return as<JsonDom>(); }
    YamlDom& yaml()        { return as<YamlDom>(); }
    TomlDom& toml()        { return as<TomlDom>(); }

    const JsonDom& ojson() const{ return as<JsonDom>(); }
    const YamlDom& yaml() const        { return as<YamlDom>(); }
    const TomlDom& toml() const        { return as<TomlDom>(); }
};

}} // namespace ofx::configbridge

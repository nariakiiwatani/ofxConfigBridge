#pragma once
#include <string_view>
#include <string>
#include <algorithm>

namespace ofx { namespace configbridge {

enum class Format { Auto, Json, OJson, Yaml, Toml, Ini, Xml };

inline Format guess_format(std::string_view path){
    auto pos = path.rfind('.');
    if (pos == std::string_view::npos) return Format::Auto;
    std::string ext(path.substr(pos+1));
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return (char)std::tolower(c); });
    if (ext=="json") return Format::Json;
    if (ext=="yaml" || ext=="yml") return Format::Yaml;
    if (ext=="toml") return Format::Toml;
    if (ext=="ini")  return Format::Ini;
    if (ext=="xml")  return Format::Xml;
    return Format::Auto;
}

struct Result {
    bool ok = true;
    const char* message = "";
    explicit operator bool() const { return ok; }
};

struct Options {
    bool auto_number_conversion = true;
    bool auto_boolean_conversion = true;
    int  float_precision = 6;
};

}} // namespace ofx::configbridge

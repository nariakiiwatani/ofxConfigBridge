#include "ofxConfigBridgeAdapterToml.hpp"
#include "ofxConfigBridgeRegistry.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>

namespace ofx { namespace configbridge {

Result AdapterToml::parseText(std::string_view text, Document& out){
    try{
        auto value = toml::parse_str<toml::ordered_type_config>(std::string(text));
        out.type = Document::Type::Toml;
        out.dom  = value;
        return {};
    }catch(const std::exception& e){
        return Result{false, e.what()};
    }
}

Result AdapterToml::loadFile(const std::string& path, Document& out){
    try{
        auto value = toml::parse<toml::ordered_type_config>(path);
        out.type = Document::Type::Toml;
        out.dom  = value;
        return {};
    }catch(const std::exception& e){
        return Result{false, e.what()};
    }
}

std::string formatFloatForToml(double value, int precision) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(precision) << value;
    std::string result = ss.str();
    
    // Remove trailing zeros, but keep at least one decimal place
    if (result.find('.') != std::string::npos) {
        while (result.length() > 2 && result.back() == '0') {
            result.pop_back();
        }
        // Ensure integers like 1 become 1.0 (keep at least one decimal place)
        if (result.back() == '.') {
            result += '0';
        }
    }
    
    return result;
}

void formatTomlValue(const toml::ordered_value& value, std::ostringstream& ss, int precision, int indent_level = 0) {
    std::string indent(indent_level * 2, ' ');
    
    switch (value.type()) {
        case toml::value_t::floating:
            ss << formatFloatForToml(value.as_floating(), precision);
            break;
        case toml::value_t::integer:
            ss << value.as_integer();
            break;
        case toml::value_t::boolean:
            ss << (value.as_boolean() ? "true" : "false");
            break;
        case toml::value_t::string:
            ss << "\"" << value.as_string() << "\"";
            break;
        case toml::value_t::array: {
            const auto& arr = value.as_array();
            ss << "[";
            for (size_t i = 0; i < arr.size(); ++i) {
                if (i > 0) ss << ", ";
                formatTomlValue(arr[i], ss, precision, indent_level);
            }
            ss << "]";
            break;
        }
        case toml::value_t::table: {
            const auto& table = value.as_table();
            bool first = true;
            for (const auto& [key, val] : table) {
                if (!first) ss << "\n";
                first = false;
                ss << indent << key << " = ";
                formatTomlValue(val, ss, precision, indent_level);
            }
            break;
        }
        default:
            // Fallback to library formatting for other types
            ss << toml::format(value);
            break;
    }
}

Result AdapterToml::dumpText(const Document& in, std::string& outText, const Options& opt){
    if (in.type != Document::Type::Toml) return Result{false, "doc type mismatch(toml)"};
    const auto& value = std::get<Document::TomlDom>(in.dom);

    std::ostringstream ss;
    formatTomlValue(value, ss, opt.float_precision);
    outText = ss.str();
    return {};
}

Result AdapterToml::saveFile(const Document& in, const std::string& path, const Options& opt){
    std::string toml_text; 
    auto r = dumpText(in, toml_text, opt); 
    if(!r) return r;
    
    std::ofstream ofs(path, std::ios::binary);
    if(!ofs) return Result{false, "open toml for write failed"};
    ofs.write(toml_text.data(), (std::streamsize)toml_text.size());
    return {};
}

// static auto-register
static struct _TomlAutoReg {
    _TomlAutoReg(){
        Registry::instance().registerAdapter(std::make_unique<AdapterToml>(Format::Toml));
        Registry::instance().registerAdapter(std::make_unique<AdapterToml>(Format::OrderedToml));
    }
} _toml_autoreg;

}} // namespace ofx::configbridge

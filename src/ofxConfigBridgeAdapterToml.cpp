#include "ofxConfigBridgeAdapterToml.hpp"
#include "ofxConfigBridgeRegistry.hpp"
#include <fstream>
#include <sstream>

namespace ofx { namespace configbridge {

Result AdapterToml::parseText(std::string_view text, Document& out, const Options&){
    try{
        auto value = toml::parse_str<toml::ordered_type_config>(std::string(text));
        out.type = Document::Type::Toml;
        out.dom  = value;
        return {};
    }catch(const std::exception& e){
        return Result{false, e.what()};
    }
}

Result AdapterToml::loadFile(const std::string& path, Document& out, const Options&){
    try{
        auto value = toml::parse<toml::ordered_type_config>(path);
        out.type = Document::Type::Toml;
        out.dom  = value;
        return {};
    }catch(const std::exception& e){
        return Result{false, e.what()};
    }
}

Result AdapterToml::dumpText(const Document& in, std::string& outText, const Options& opt){
    if (in.type != Document::Type::Toml) return Result{false, "doc type mismatch(toml)"};
    const auto& value = std::get<Document::TomlDom>(in.dom);

    std::ostringstream ss;
    ss << toml::format(value);
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

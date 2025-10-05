#include "ofxConfigBridgeAdapterYamlCpp.hpp"
#include "ofxConfigBridgeRegistry.hpp"
#include <fstream>
#include <sstream>

namespace ofx { namespace configbridge {

Result AdapterYamlCpp::parseText(std::string_view text, Document& out, const Options&){
    try{
        Document::YamlDom node = YAML::Load(std::string(text));
        out.type = Document::Type::Yaml;
        out.dom  = std::move(node);
        return {};
    }catch(const std::exception& e){
        return Result{false, e.what()};
    }
}
Result AdapterYamlCpp::loadFile(const std::string& path, Document& out, const Options&){
    try{
        Document::YamlDom node = YAML::LoadFile(path);
        out.type = Document::Type::Yaml;
        out.dom  = std::move(node);
        return {};
    }catch(const std::exception& e){
        return Result{false, e.what()};
    }
}
Result AdapterYamlCpp::dumpText(const Document& in, std::string& outText, const Options&){
    if (in.type != Document::Type::Yaml) return Result{false, "doc type mismatch(yaml)"};
    const auto& node = std::get<Document::YamlDom>(in.dom);
    YAML::Emitter emitter;
    emitter << node;
    if (!emitter.good()) return Result{false, "yaml emitter error"};
    outText = emitter.c_str();
    return {};
}
Result AdapterYamlCpp::saveFile(const Document& in, const std::string& path, const Options& opt){
    std::string y; auto r = dumpText(in, y, opt); if(!r) return r;
    std::ofstream ofs(path, std::ios::binary);
    if(!ofs) return Result{false, "open yaml for write failed"};
    ofs.write(y.data(), (std::streamsize)y.size());
    return {};
}

// static auto-register
static struct _YamlAutoReg {
    _YamlAutoReg(){ Registry::instance().registerAdapter(std::make_unique<AdapterYamlCpp>()); }
} _yaml_autoreg;

}} // namespace ofx::configbridge

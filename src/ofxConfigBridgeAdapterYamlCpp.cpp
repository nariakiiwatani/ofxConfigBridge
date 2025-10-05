#include "ofxConfigBridgeAdapterYamlCpp.hpp"
#include "ofxConfigBridgeRegistry.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>

namespace ofx { namespace configbridge {

Result AdapterYamlCpp::parseText(std::string_view text, Document& out){
    try{
        Document::YamlDom node = YAML::Load(std::string(text));
        out.type = Document::Type::Yaml;
        out.dom  = std::move(node);
        return {};
    }catch(const std::exception& e){
        return Result{false, e.what()};
    }
}
Result AdapterYamlCpp::loadFile(const std::string& path, Document& out){
    try{
        Document::YamlDom node = YAML::LoadFile(path);
        out.type = Document::Type::Yaml;
        out.dom  = std::move(node);
        return {};
    }catch(const std::exception& e){
        return Result{false, e.what()};
    }
}

std::string formatFloatForYaml(double value, int precision) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(precision) << value;
    return ss.str();
}

YAML::Node applyPrecisionToYamlNode(const YAML::Node& node, int precision) {
    YAML::Node result;
    
    switch (node.Type()) {
        case YAML::NodeType::Scalar:
            if (node.IsScalar()) {
                try {
                    // Try to convert to double
                    double val = node.as<double>();
                    result = formatFloatForYaml(val, precision);
                } catch (...) {
                    // Not a number, keep as is
                    result = node;
                }
            }
            break;
            
        case YAML::NodeType::Sequence:
            for (std::size_t i = 0; i < node.size(); ++i) {
                result[i] = applyPrecisionToYamlNode(node[i], precision);
            }
            break;
            
        case YAML::NodeType::Map:
            for (const auto& pair : node) {
                result[pair.first.as<std::string>()] = applyPrecisionToYamlNode(pair.second, precision);
            }
            break;
            
        default:
            result = node;
            break;
    }
    
    return result;
}

Result AdapterYamlCpp::dumpText(const Document& in, std::string& outText, const Options& opt){
    if (in.type != Document::Type::Yaml) return Result{false, "doc type mismatch(yaml)"};
    const auto& node = std::get<Document::YamlDom>(in.dom);
    
    // Apply precision control to the entire YAML tree
    YAML::Node processedNode = applyPrecisionToYamlNode(node, opt.float_precision);
    
    YAML::Emitter emitter;
    emitter << processedNode;
    
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

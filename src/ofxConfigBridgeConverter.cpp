#include "ofxConfigBridgeConverter.hpp"
#include "ofxConfigBridgeAdapter.hpp"
#include "ofxConfigBridgeRegistry.hpp"
#include <fstream>
#include <sstream>

namespace ofx { namespace configbridge {

Converter& Converter::instance(){ static Converter c; return c; }

void Converter::registerTextBridge(Format from, Format to, TextBridge fn){
    text_[key(from,to)] = std::move(fn);
}
void Converter::registerNativeBridge(Format from, Format to, NativeBridge fn){
    native_[key(from,to)] = std::move(fn);
}

Result Converter::convert(const Document& in, Format to, Document& out, const Options& opt){
    Format from =
        in.type==Document::Type::Json ? Format::Json :
		in.type==Document::Type::Yaml ? Format::Yaml :
		in.type==Document::Type::Toml ? Format::Toml :
        Format::Auto;

    auto itN = native_.find(key(from, to));
    if (itN != native_.end()) return itN->second(in, out, opt);

    // If no native bridge is available, this conversion is not supported
    return Result{false, "direct format conversion not supported"};
}

Result Converter::convertText(Format from, Format to, const std::string& in, std::string& out, const Options& opt){
    auto it = text_.find(key(from,to));
    if (it == text_.end()) {
        auto* aFrom = Registry::instance().find(from);
        auto* aTo   = Registry::instance().find(to);
        if (!aFrom || !aTo) return {false, "no adapter for bridge"};
        Document d;
        auto r1 = aFrom->parseText(in, d);
        if (!r1) return r1;
        return aTo->dumpText(d, out, opt);
    }
    return it->second(in, out, opt);
}

Result Converter::convertFile(const std::string& inPath, Format inFmt,
                              const std::string& outPath, Format outFmt, const Options& opt){
    std::ifstream ifs(inPath, std::ios::binary);
    if(!ifs) return {false, "open input failed"};
    std::ostringstream oss; oss<<ifs.rdbuf();
    std::string in = oss.str(), out;
    auto r = convertText(inFmt, outFmt, in, out, opt);
    if(!r) return r;
    std::ofstream ofs(outPath, std::ios::binary);
    if(!ofs) return {false, "open output failed"};
    ofs.write(out.data(), (std::streamsize)out.size());
    return {};
}

}} // namespace ofx::configbridge6

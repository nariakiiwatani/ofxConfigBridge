#pragma once
#include "ofxConfigBridgeFormat.hpp"
#include "ofxConfigBridgeDocument.hpp"
#include "ofxConfigBridgeRegistry.hpp"
#include <functional>
#include <unordered_map>
#include <string>

namespace ofx { namespace configbridge {

using TextBridge   = std::function<Result(const std::string& in, std::string& out, const Options& opt)>;
using NativeBridge = std::function<Result(const Document& inDoc, Document& outDoc, const Options& opt)>;

class Converter {
public:
    static Converter& instance();

    void registerTextBridge(Format from, Format to, TextBridge fn);
    void registerNativeBridge(Format from, Format to, NativeBridge fn);

    Result convert(const Document& in, Format to, Document& out, const Options& opt);
    Result convertText(Format from, Format to, const std::string& in, std::string& out, const Options& opt);
    Result convertFile(const std::string& inPath, Format inFmt,
                       const std::string& outPath, Format outFmt, const Options& opt);

private:
    std::unordered_map<long long, TextBridge>   text_;
    std::unordered_map<long long, NativeBridge> native_;
    static long long key(Format a, Format b){
        auto normalizeFormat = [](Format f) -> Format {
            switch(f) {
                case Format::OrderedJson: return Format::Json;
                case Format::OrderedToml: return Format::Toml;
                default: return f;
            }
        };
        
        Format normalizedA = normalizeFormat(a);
        Format normalizedB = normalizeFormat(b);
        return (long long)static_cast<int>(normalizedA)<<32 | (int)normalizedB;
    }
};

}} // namespace ofx::configbridge

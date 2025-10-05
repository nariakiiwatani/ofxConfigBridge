#pragma once
#include "ofxConfigBridgeAdapter.hpp"

namespace ofx { namespace configbridge {

class AdapterJson final : public IAdapter {
private:
    Format output_format;
public:
    AdapterJson(Format fmt) : output_format(fmt) {}
    
    Format format() const override { return output_format; }
    const char* name() const override {
        return output_format == Format::Json ? "json-nlohmann" : "json-nlohmann-ordered";
    }

    Result parseText(std::string_view text, Document& out, const Options& opt) override;
    Result loadFile (const std::string& path, Document& out, const Options& opt) override;
    Result dumpText (const Document& in, std::string& outText, const Options& opt) override;
    Result saveFile (const Document& in, const std::string& path, const Options& opt) override;
};

}} // namespace ofx::configbridge

#pragma once
#include "ofxConfigBridgeAdapter.hpp"
#include "toml.hpp"

namespace ofx { namespace configbridge {

class AdapterToml final : public IAdapter {
private:
    Format format_;
public:
    AdapterToml(Format f = Format::Toml) : format_(f) {}
    Format format() const override { return format_; }
    const char* name() const override { return format_ == Format::Toml ? "toml11(ordered)" : "toml11(unordered)"; }

    Result parseText(std::string_view text, Document& out) override;
    Result loadFile (const std::string& path, Document& out) override;
    Result dumpText (const Document& in, std::string& outText, const Options& opt) override;
    Result saveFile (const Document& in, const std::string& path, const Options& opt) override;
};

}} // namespace ofx::configbridge

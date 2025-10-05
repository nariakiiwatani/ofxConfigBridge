#pragma once
#include "ofxConfigBridgeFormat.hpp"
#include "ofxConfigBridgeDocument.hpp"
#include <string>
#include <string_view>

namespace ofx { namespace configbridge {

class IAdapter {
public:
    virtual ~IAdapter() = default;
    virtual Format format() const = 0;
    virtual const char* name() const = 0;

    virtual Result parseText(std::string_view text, Document& out) = 0;
    virtual Result loadFile (const std::string& path, Document& out) = 0;

    virtual Result dumpText (const Document& in, std::string& outText, const Options& opt) = 0;
    virtual Result saveFile (const Document& in, const std::string& path, const Options& opt) = 0;
};

}} // namespace ofx::configbridge

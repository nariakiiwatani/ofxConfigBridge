#pragma once
#include "ofxConfigBridgeAdapter.hpp"

namespace ofx { namespace configbridge {
class AdapterOJson final : public IAdapter {
public:
  Format format() const override { return Format::OJson; }
  const char* name() const override { return "json-nlohmann-ordered"; }
  Result parseText(std::string_view text, Document& out, const Options&) override {
	try{
	  nlohmann::ordered_json j = nlohmann::ordered_json::parse(text);
	  out.type = Document::Type::OrderedJson; out.dom = std::move(j); return {};
	}catch(const std::exception& e){ return {false, e.what()}; }
  }
  Result dumpText(const Document& in, std::string& outText, const Options&) override {
	if (in.type == Document::Type::OrderedJson) { outText = std::get<nlohmann::ordered_json>(in.dom).dump(2); return {}; }
	return {false, "doc type mismatch(ojson)"};
  }
  Result loadFile(const std::string& path, Document& out, const Options& opt) override;
  Result saveFile(const Document& in, const std::string& path, const Options& opt) override;
};
}} // namespace ofx::configbridge

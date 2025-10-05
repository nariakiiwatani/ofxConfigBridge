#include "ofxConfigBridgeAdapterJson.hpp"
#include "ofxConfigBridgeRegistry.hpp"
#include <fstream>
#include <sstream>

namespace ofx { namespace configbridge {

Result AdapterJson::parseText(std::string_view text, Document& out, const Options& opt){
	try{
		auto oj = nlohmann::ordered_json::parse(text);
		out.type = Document::Type::Json;
		out.dom = std::move(oj);
		return {};
	}catch(const std::exception& e){ return {false, e.what()};}
}
Result AdapterJson::loadFile(const std::string& path, Document& out, const Options& opt){
	std::ifstream ifs(path, std::ios::binary);
	if(!ifs) return Result{false, "open json failed"};
	std::ostringstream oss; oss<<ifs.rdbuf();
	return parseText(oss.str(), out, opt);
}
Result AdapterJson::dumpText(const Document& in, std::string& outText, const Options&) {
	if (in.type != Document::Type::Json) return {false, "doc type mismatch"};
	
	const auto& oj = std::get<nlohmann::ordered_json>(in.dom);
	if (output_format == Format::Json) {
		nlohmann::json j = nlohmann::json(oj);
		outText = j.dump(2);
	} else {
		outText = oj.dump(2);
	}
	return {};
}
Result AdapterJson::saveFile(const Document& in, const std::string& path, const Options& opt){
	std::string s; auto r = dumpText(in, s, opt); if(!r) return r;
	std::ofstream ofs(path, std::ios::binary);
	if(!ofs) return Result{false, "open json for write failed"};
	ofs.write(s.data(), (std::streamsize)s.size());
	return {};
}

static struct _JsonAutoReg {
	_JsonAutoReg(){
		Registry::instance().registerAdapter(std::make_unique<AdapterJson>(Format::Json));
		Registry::instance().registerAdapter(std::make_unique<AdapterJson>(Format::OrderedJson));
	}
} _json_autoreg;
}} // namespace ofx::configbridge

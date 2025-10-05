#include "ofxConfigBridgeAdapterJson.hpp"
#include "ofxConfigBridgeRegistry.hpp"
#include <fstream>
#include <sstream>

namespace ofx { namespace configbridge {

Result AdapterJson::parseText(std::string_view text, Document& out, const Options& opt){
	try{
		nlohmann::json j = nlohmann::json::parse(text);
		out.type = Document::Type::Json; out.dom = std::move(j); return {};
	}catch(const std::exception& e){ return {false, e.what()};}
}
Result AdapterJson::loadFile(const std::string& path, Document& out, const Options& opt){
	std::ifstream ifs(path, std::ios::binary);
	if(!ifs) return Result{false, "open json failed"};
	std::ostringstream oss; oss<<ifs.rdbuf();
	return parseText(oss.str(), out, opt);
}
Result AdapterJson::dumpText(const Document& in, std::string& outText, const Options&) {
	if (in.type == Document::Type::Json) { outText = std::get<nlohmann::json>(in.dom).dump(2); return {}; }
	return {false, "doc type mismatch(json)"};
}
Result AdapterJson::saveFile(const Document& in, const std::string& path, const Options& opt){
	std::string s; auto r = dumpText(in, s, opt); if(!r) return r;
	std::ofstream ofs(path, std::ios::binary);
	if(!ofs) return Result{false, "open json for write failed"};
	ofs.write(s.data(), (std::streamsize)s.size());
	return {};
}

static struct _JsonAutoReg {
	_JsonAutoReg(){ Registry::instance().registerAdapter(std::make_unique<AdapterJson>()); }
} _json_autoreg;
}} // namespace ofx::configbridge

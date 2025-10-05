#include "ofxConfigBridgeAdapterJson.hpp"
#include "ofxConfigBridgeRegistry.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <functional>

namespace ofx { namespace configbridge {

Result AdapterJson::parseText(std::string_view text, Document& out){
	try{
		auto oj = nlohmann::ordered_json::parse(text);
		out.type = Document::Type::Json;
		out.dom = std::move(oj);
		return {};
	}catch(const std::exception& e){ return {false, e.what()};}
}
Result AdapterJson::loadFile(const std::string& path, Document& out){
	std::ifstream ifs(path, std::ios::binary);
	if(!ifs) return Result{false, "open json failed"};
	std::ostringstream oss; oss<<ifs.rdbuf();
	return parseText(oss.str(), out);
}
Result AdapterJson::dumpText(const Document& in, std::string& outText, const Options& opt) {
	if (in.type != Document::Type::Json) return {false, "doc type mismatch"};
	
	const auto& oj = std::get<nlohmann::ordered_json>(in.dom);
	
	std::function<void(const nlohmann::ordered_json&, std::ostringstream&, int)> serializeJson;
	serializeJson = [&](const nlohmann::ordered_json& obj, std::ostringstream& ss, int indent) {
		std::string spaces(indent, ' ');
		
		if (obj.is_object()) {
			ss << "{\n";
			bool first = true;
			for (auto it = obj.begin(); it != obj.end(); ++it) {
				if (!first) ss << ",\n";
				first = false;
				ss << spaces << "  \"" << it.key() << "\": ";
				serializeJson(it.value(), ss, indent + 2);
			}
			ss << "\n" << spaces << "}";
		} else if (obj.is_array()) {
			ss << "[\n";
			bool first = true;
			for (const auto& elem : obj) {
				if (!first) ss << ",\n";
				first = false;
				ss << spaces << "  ";
				serializeJson(elem, ss, indent + 2);
			}
			ss << "\n" << spaces << "]";
		} else if (obj.is_number_float()) {
			std::ostringstream temp_ss;
			temp_ss << std::fixed << std::setprecision(opt.float_precision) << obj.get<double>();
			std::string float_str = temp_ss.str();
			
			// Remove trailing zeros, but keep at least one decimal place
			if (float_str.find('.') != std::string::npos) {
				while (float_str.length() > 2 && float_str.back() == '0') {
					float_str.pop_back();
				}
				// Ensure integers like 1 become 1.0 (keep at least one decimal place)
				if (float_str.back() == '.') {
					float_str += '0';
				}
			}
			
			ss << float_str;
		} else if (obj.is_string()) {
			ss << "\"" << obj.get<std::string>() << "\"";
		} else if (obj.is_boolean()) {
			ss << (obj.get<bool>() ? "true" : "false");
		} else if (obj.is_null()) {
			ss << "null";
		} else if (obj.is_number_integer()) {
			ss << obj.get<int64_t>();
		} else if (obj.is_number_unsigned()) {
			ss << obj.get<uint64_t>();
		}
	};
	
	std::ostringstream ss;
	if (output_format == Format::Json) {
		nlohmann::json j = nlohmann::json(oj);
		serializeJson(nlohmann::ordered_json(j), ss, 0);
	} else {
		serializeJson(oj, ss, 0);
	}
	
	outText = ss.str();
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

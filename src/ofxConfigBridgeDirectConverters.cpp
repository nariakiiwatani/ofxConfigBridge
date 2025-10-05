#include "ofxConfigBridgeConverter.hpp"
#include <yaml-cpp/yaml.h>
#include <nlohmann/json.hpp>
#include <string>

namespace {
inline bool tag_is(const YAML::Node& n, const char* tag) {
	try { return n.Tag() == tag; } catch (...) { return false; }
}
inline std::string scalar_of(const YAML::Node& n) {
	try { return n.Scalar(); } catch (...) { return std::string(); }
}
inline bool is_null_token(const std::string& s){
	return s == "~" || s == "null" || s == "Null" || s == "NULL";
}
inline static std::string format_float_preserving_decimal(double v, int float_precision){
	std::ostringstream oss;
	if (std::isfinite(v) && std::floor(v) == v){
		oss.setf(std::ios::fixed);
		oss << std::setprecision(1) << v;
	} else {
		int prec = (float_precision > 0 ? float_precision : 6);
		oss << std::defaultfloat << std::setprecision(prec) << v;
	}
	return oss.str();
}

}
namespace ofx { namespace configbridge {

template<class BasicJson>
static BasicJson YamlScalarToJson(const YAML::Node& n) {
	using J = BasicJson;

	if (tag_is(n, "!!str")) return J(scalar_of(n));

	try { return J(n.as<bool>()); } catch (...) {}
	try { return J(n.as<long long>()); } catch (...) {}
	try { return J(n.as<unsigned long long>()); } catch (...) {}
	try { return J(n.as<double>()); } catch (...) {}

	const auto s = scalar_of(n);
	if (is_null_token(s)) return nullptr;

	return J(s);
}

static nlohmann::json YAMLNodeToJson(const YAML::Node& n, const Options& /*opt*/){
	using json = nlohmann::json;
	if (n.IsMap()){ json o=json::object(); for(auto it=n.begin(); it!=n.end(); ++it) o[it->first.as<std::string>()]=YAMLNodeToJson(it->second, {}); return o; }
	if (n.IsSequence()){ json a=json::array(); for(auto it=n.begin(); it!=n.end(); ++it) a.push_back(YAMLNodeToJson(*it, {})); return a; }
	if (n.IsNull()) return nullptr;
	return YamlScalarToJson<json>(n);
}

static nlohmann::ordered_json YAMLNodeToOJson(const YAML::Node& n, const Options& /*opt*/){
	using ojson = nlohmann::ordered_json;
	if (n.IsMap()){ ojson o=ojson::object(); for(auto it=n.begin(); it!=n.end(); ++it) o[it->first.as<std::string>()]=YAMLNodeToOJson(it->second, {}); return o; }
	if (n.IsSequence()){ ojson a=ojson::array(); for(auto it=n.begin(); it!=n.end(); ++it) a.push_back(YAMLNodeToOJson(*it, {})); return a; }
	if (n.IsNull()) return nullptr;
	return YamlScalarToJson<ojson>(n);
}

template<class BasicJson>
static void JsonToYAMLNode(YAML::Node& node, const BasicJson& j, const Options& opt){
	if (j.is_object()){
		node = YAML::Node(YAML::NodeType::Map);
		for (auto it=j.begin(); it!=j.end(); ++it){ YAML::Node c; JsonToYAMLNode(c, it.value(), {}); node[it.key()] = c; }
	} else if (j.is_array()){
		node = YAML::Node(YAML::NodeType::Sequence);
		for (auto& v : j){ YAML::Node c; JsonToYAMLNode(c, v, {}); node.push_back(c); }
	} else if (j.is_boolean()){
		node = YAML::Node(j.template get<bool>());
	} else if (j.is_number_integer()){
		node = YAML::Node(j.template get<long long>());
	} else if (j.is_number_unsigned()){
		node = YAML::Node(j.template get<unsigned long long>());
	} else if (j.is_number_float()){
		const double v = j.template get<double>();
		if (std::isfinite(v) && std::floor(v) == v) {
			std::string s = format_float_preserving_decimal(v, opt.float_precision);
			node = YAML::Node(s);
		} else {
			node = YAML::Node(v);
		}
	} else if (j.is_null()){
		node = YAML::Node();
	} else {
		node = YAML::Node(j.template get<std::string>());
	}
}

static struct _RegisterDirect {
	_RegisterDirect(){
		// YAML -> JSON (unordered)
		Converter::instance().registerNativeBridge(Format::Yaml, Format::Json,
		  [](const Document& in, Document& out, const Options& opt)->Result{
			if (in.type != Document::Type::Yaml) return {false,"in is not yaml"};
			const auto& y = std::get<Document::YamlDom>(in.dom);
			nlohmann::json j = YAMLNodeToJson(y, opt);
			out.type = Document::Type::Json; out.dom = std::move(j); return {};
		  });

		// YAML -> OJSON (ordered)
		Converter::instance().registerNativeBridge(Format::Yaml, Format::OJson,
		  [](const Document& in, Document& out, const Options& opt)->Result{
			if (in.type != Document::Type::Yaml) return {false,"in is not yaml"};
			const auto& y = std::get<Document::YamlDom>(in.dom);
			nlohmann::ordered_json j = YAMLNodeToOJson(y, opt);
			out.type = Document::Type::OrderedJson; out.dom = std::move(j); return {};
		  });

		// JSON (unordered) -> YAML
		Converter::instance().registerNativeBridge(Format::Json, Format::Yaml,
		  [](const Document& in, Document& out, const Options& opt)->Result{
			if (in.type != Document::Type::Json) return {false,"in is not json"};
			const auto& j = std::get<nlohmann::json>(in.dom);
			YAML::Node y; JsonToYAMLNode(y, j, opt);
			out.type = Document::Type::Yaml; out.dom = std::move(y); return {};
		  });

		// OJSON (ordered) -> YAML
		Converter::instance().registerNativeBridge(Format::OJson, Format::Yaml,
		  [](const Document& in, Document& out, const Options& opt)->Result{
			if (in.type != Document::Type::OrderedJson) return {false,"in is not ojson"};
			const auto& j = std::get<nlohmann::ordered_json>(in.dom);
			YAML::Node y; JsonToYAMLNode(y, j, opt);
			out.type = Document::Type::Yaml; out.dom = std::move(y); return {};
		  });

		// JSON -> OJSON
		Converter::instance().registerNativeBridge(Format::Json, Format::OJson,
		  [](const Document& in, Document& out, const Options&)->Result{
			if (in.type != Document::Type::Json) return {false,"not json"};
			nlohmann::ordered_json oj = nlohmann::ordered_json(std::get<nlohmann::json>(in.dom));
			out.type = Document::Type::OrderedJson; out.dom = std::move(oj); return {};
		  });
		// OJSON -> JSON
		Converter::instance().registerNativeBridge(Format::OJson, Format::Json,
		  [](const Document& in, Document& out, const Options&)->Result{
			if (in.type != Document::Type::OrderedJson) return {false,"not ojson"};
			nlohmann::json j = nlohmann::json(std::get<nlohmann::ordered_json>(in.dom));
			out.type = Document::Type::Json; out.dom = std::move(j); return {};
		  });
	}
} _direct;

}} // namespace ofx::configbridge

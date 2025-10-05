#include "ofxConfigBridgeConverter.hpp"
#include <yaml-cpp/yaml.h>
#include <nlohmann/json.hpp>
#include <string>

namespace ofx { namespace configbridge {

inline static bool TagIs(const YAML::Node& n, const char* tag) {
	try { return n.Tag() == tag; } catch (...) { return false; }
}
inline static std::string ScalarOf(const YAML::Node& n) {
	try { return n.Scalar(); } catch (...) { return std::string(); }
}

template<class BasicJson>
static BasicJson YamlScalarToJson(const YAML::Node& n, const Options& opt) {
	using J = BasicJson;

	if (TagIs(n, "!!str")) {
		return J(ScalarOf(n));
	}

	const std::string s = ScalarOf(n);

	if(!opt.auto_boolean_conversion) {
		if (TagIs(n, "!!bool")) {
			try { return J(n.as<bool>()); } catch (...) {}
		}
		if (s == "true")  return J(true);
		if (s == "false") return J(false);
	} else {
		try { return J(n.as<bool>()); } catch (...) {}
	}

	if(!opt.auto_number_conversion) {
		if (TagIs(n, "!!int")) {
			try { return J(n.as<long long>()); } catch (...) {}
			try { return J(n.as<unsigned long long>()); } catch (...) {}
		}
		if (TagIs(n, "!!float")) {
			try { return J(n.as<double>()); } catch (...) {}
		}
	} else {
		try { return J(n.as<long long>()); } catch (...) {}
		try { return J(n.as<unsigned long long>()); } catch (...) {}
		try { return J(n.as<double>()); } catch (...) {}
	}

	if (s == "~" || s == "null" || s == "Null" || s == "NULL") {
		return nullptr;
	}

	return J(s);
}

static nlohmann::json YAMLNodeToJson(const YAML::Node& n, const Options& opt){
	using json = nlohmann::json;
	if (n.IsMap()){
		json obj = json::object();
		for (auto it = n.begin(); it != n.end(); ++it){
			std::string key = it->first.as<std::string>();
			obj[key] = YAMLNodeToJson(it->second, opt);
		}
		return obj;
	}
	if (n.IsSequence()){
		json arr = json::array();
		for (auto it = n.begin(); it != n.end(); ++it){
			arr.push_back(YAMLNodeToJson(*it, opt));
		}
		return arr;
	}
	if (n.IsNull()) return nullptr;

	return YamlScalarToJson<json>(n, opt);
}

static nlohmann::ordered_json YAMLNodeToOJson(const YAML::Node& n, const Options& opt){
	using ojson = nlohmann::ordered_json;
	if (n.IsMap()){
		ojson obj = ojson::object();
		for (auto it = n.begin(); it != n.end(); ++it){
			obj[it->first.as<std::string>()] = YAMLNodeToOJson(it->second, opt);
		}
		return obj;
	}
	if (n.IsSequence()){
		ojson arr = ojson::array();
		for (auto it = n.begin(); it != n.end(); ++it){
			arr.push_back(YAMLNodeToOJson(*it, opt));
		}
		return arr;
	}
	if (n.IsNull()) return nullptr;

	return YamlScalarToJson<ojson>(n, opt);
}

template<class BasicJson>
static void JsonToYAMLNode(YAML::Node& node, const BasicJson& j, const Options& opt){
	if (j.is_object()){
		node = YAML::Node(YAML::NodeType::Map);
		for (auto it = j.begin(); it != j.end(); ++it){
			YAML::Node child;
			JsonToYAMLNode(child, it.value(), opt);
			node[it.key()] = child;
		}
	} else if (j.is_array()){
		node = YAML::Node(YAML::NodeType::Sequence);
		for (auto& v : j){
			YAML::Node child;
			JsonToYAMLNode(child, v, opt);
			node.push_back(child);
		}
	} else if (j.is_boolean()){
		node = YAML::Node(j.template get<bool>());
	} else if (j.is_number_integer()){
		node = YAML::Node(j.template get<long long>());
	} else if (j.is_number_unsigned()){
		node = YAML::Node(j.template get<unsigned long long>());
	} else if (j.is_number_float()){
		node = YAML::Node(j.template get<double>());
	} else if (j.is_null()){
		node = YAML::Node(); // null
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

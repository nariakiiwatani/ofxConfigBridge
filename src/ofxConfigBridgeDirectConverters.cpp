#include "ofxConfigBridgeConverter.hpp"
#include "yaml.h"
#include <nlohmann/json.hpp>
#include "toml.hpp"
#include <string>
#include <sstream>

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
		for (auto it=j.begin(); it!=j.end(); ++it){ YAML::Node c; JsonToYAMLNode(c, it.value(), opt); node[it.key()] = c; }
	} else if (j.is_array()){
		node = YAML::Node(YAML::NodeType::Sequence);
		for (auto& v : j){ YAML::Node c; JsonToYAMLNode(c, v, opt); node.push_back(c); }
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

template<class BasicJson>
static BasicJson TomlValueToJson(const toml::ordered_value& v) {
	using J = BasicJson;

	if (v.is_string())        return J(v.as_string());
	else if (v.is_integer())  return J(v.as_integer());
	else if (v.is_floating()) return J(v.as_floating());
	else if (v.is_boolean())  return J(v.as_boolean());
	else if (v.is_array()) {
		J arr = J::array();
		for (int i = 0; i < v.size(); ++i) {
			arr.push_back(TomlValueToJson<J>(v.at(i)));
		}
		return arr;
	}
	else if (v.is_table()) {
		J obj = J::object();
		auto t = toml::get<toml::ordered_table>(v);
		for(const auto &[key, value] : t) {
			obj[key] = TomlValueToJson<J>(value);
		}
		return obj;
	}
	else if (v.is_local_date() || v.is_local_time()
		  || v.is_local_datetime() || v.is_offset_datetime()) {
		return J(toml::format(v));
	}
	return J(nullptr);
}

template<class BasicJson>
static void JsonToTomlValue(toml::ordered_table& table, const std::string& key, const BasicJson& json) {
	if (json.is_null()) {
		table[key] = toml::ordered_value("");
	} else if (json.is_boolean()) {
		table[key] = toml::ordered_value(json.template get<bool>());
	} else if (json.is_number_integer()) {
		table[key] = toml::ordered_value((std::int64_t)json.template get<std::int64_t>());
	} else if (json.is_number_unsigned()) {
		const auto u = json.template get<std::uint64_t>();
		if (u <= static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()))
			table[key] = toml::ordered_value((std::int64_t)u);
		else
			table[key] = toml::ordered_value((double)u);
	} else if (json.is_number_float()) {
		table[key] = toml::ordered_value(json.template get<double>());
	} else if (json.is_string()) {
		table[key] = toml::ordered_value(json.template get<std::string>());
	} else if (json.is_array()) {
		toml::array arr;
		for (const auto& item : json) {
			if (item.is_string())               arr.push_back(toml::ordered_value(item.template get<std::string>()));
			else if (item.is_number_integer())  arr.push_back(toml::ordered_value((std::int64_t)item.template get<std::int64_t>()));
			else if (item.is_number_unsigned()) {
				const auto u2 = item.template get<std::uint64_t>();
				if (u2 <= static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()))
					arr.push_back(toml::ordered_value((std::int64_t)u2));
				else
					arr.push_back(toml::ordered_value((double)u2));
			}
			else if (item.is_number_float())    arr.push_back(toml::ordered_value(item.template get<double>()));
			else if (item.is_boolean())         arr.push_back(toml::ordered_value(item.template get<bool>()));
			else if (item.is_object()) {
				toml::ordered_table nested;
				for (const auto& [nk, nv] : item.items())
					JsonToTomlValue(nested, nk, nv);
				arr.push_back(toml::ordered_value(nested));
			} else if (item.is_array()) {
				toml::array arr2;
				for (const auto& sub : item) {
					if (sub.is_string())               arr2.push_back(toml::ordered_value(sub.template get<std::string>()));
					else if (sub.is_number_integer())  arr2.push_back(toml::ordered_value((std::int64_t)sub.template get<std::int64_t>()));
					else if (sub.is_number_unsigned()) {
						const auto u3 = sub.template get<std::uint64_t>();
						if (u3 <= static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()))
							arr2.push_back(toml::ordered_value((std::int64_t)u3));
						else
							arr2.push_back(toml::ordered_value((double)u3));
					}
					else if (sub.is_number_float())    arr2.push_back(toml::ordered_value(sub.template get<double>()));
					else if (sub.is_boolean())         arr2.push_back(toml::ordered_value(sub.template get<bool>()));
				}
				arr.push_back(toml::ordered_value(arr2));
			}
		}
		table[key] = toml::ordered_value(arr);
	} else if (json.is_object()) {
		toml::ordered_table nested;
		for (const auto& [nk, nv] : json.items())
			JsonToTomlValue(nested, nk, nv);
		table[key] = toml::ordered_value(nested);
	} else {
		table[key] = toml::ordered_value("");
	}
}

template<class BasicJson>
static void JsonToTomlTable(toml::ordered_table& table, const BasicJson& json, const Options& /*opt*/) {
	for (const auto& [key, value] : json.items())
		JsonToTomlValue(table, key, value);
}

static YAML::Node TomlValueToYaml(const toml::ordered_value& v) {
	YAML::Node y;
	if (v.is_string())            y = YAML::Node(toml::get<std::string>(v));
	else if (v.is_integer())      y = YAML::Node((long long)toml::get<std::int64_t>(v));
	else if (v.is_floating())     y = YAML::Node((double)toml::get<double>(v));
	else if (v.is_boolean())      y = YAML::Node(toml::get<bool>(v));
	else if (v.is_array()) {
		y = YAML::Node(YAML::NodeType::Sequence);
		for (const auto& el : toml::get<toml::array>(v))
			y.push_back(TomlValueToYaml(el));
	}
	else if (v.is_table()) {
		y = YAML::Node(YAML::NodeType::Map);
		for (const auto& kv : toml::get<toml::ordered_table>(v))
			y[kv.first] = TomlValueToYaml(kv.second);
	}
	else if (v.is_local_date() || v.is_local_time()
		  || v.is_local_datetime() || v.is_offset_datetime()) {
		y = YAML::Node(toml::format(v));
	}
	else y = YAML::Node(); // null
	return y;
}

static toml::ordered_value YamlNodeToToml(const YAML::Node& n) {
	if (n.IsMap()) {
		toml::ordered_table table;
		for (auto it = n.begin(); it != n.end(); ++it) {
			const std::string key = it->first.as<std::string>();
			table[key] = YamlNodeToToml(it->second);
		}
		return toml::ordered_value(table);
	}
	else if (n.IsSequence()) {
		toml::array arr;
		for (auto it = n.begin(); it != n.end(); ++it) {
			arr.push_back(YamlNodeToToml(*it));
		}
		return toml::ordered_value(arr);
	}
	else if (n.IsNull()) {
		return toml::ordered_value("");
	}
	else if (n.IsScalar()) {
		if (tag_is(n, "!!str")) {
			return toml::ordered_value(scalar_of(n));
		}

		try { return toml::ordered_value(n.as<bool>()); } catch (...) {}
		try { return toml::ordered_value((std::int64_t)n.as<long long>()); } catch (...) {}
		try { return toml::ordered_value(n.as<double>()); } catch (...) {}

		const auto s = scalar_of(n);
		if (is_null_token(s)) {
			return toml::ordered_value("");  // null
		}
		return toml::ordered_value(s);
	}
	
	return toml::ordered_value("");
}
static struct _RegisterDirect {
	_RegisterDirect(){
		// OrderedJSON ⇔ YAML
		Converter::instance().registerNativeBridge(Format::Json, Format::Yaml,
			  [](const Document& in, Document& out, const Options& opt)->Result{
			if (in.type != Document::Type::Json) return {false,"in is not ojson"};
			const auto& j = std::get<nlohmann::ordered_json>(in.dom);
			YAML::Node y; JsonToYAMLNode(y, j, opt);
			out.type = Document::Type::Yaml; out.dom = std::move(y); return {};
		});
		
		Converter::instance().registerNativeBridge(Format::Yaml, Format::Json,
			  [](const Document& in, Document& out, const Options& opt)->Result{
			if (in.type != Document::Type::Yaml) return {false,"in is not yaml"};
			const auto& y = std::get<Document::YamlDom>(in.dom);
			nlohmann::ordered_json j = YAMLNodeToOJson(y, opt);
			out.type = Document::Type::Json; out.dom = std::move(j); return {};
		});

		// OrderedJSON ⇔ TOML
		Converter::instance().registerNativeBridge(Format::Json, Format::Toml,
			  [](const Document& in, Document& out, const Options& opt)->Result{
			if (in.type != Document::Type::Json) return {false,"in is not ojson"};
			const auto& j = std::get<nlohmann::ordered_json>(in.dom);
			toml::ordered_table t; JsonToTomlTable(t, j, opt);
			out.type = Document::Type::Toml; out.dom = std::move(t); return {};
		});
		
		Converter::instance().registerNativeBridge(Format::Toml, Format::Json,
			  [](const Document& in, Document& out, const Options& opt)->Result{
			if (in.type != Document::Type::Toml) return {false,"in is not toml"};
			const auto& v = std::get<Document::TomlDom>(in.dom);
			if (!v.is_table()) return {false, "toml root is not table"};
			auto j = TomlValueToJson<nlohmann::ordered_json>(v);
			out.type = Document::Type::Json; out.dom = std::move(j); return {};
		});

		// TOML -> YAML
		Converter::instance().registerNativeBridge(Format::Toml, Format::Yaml,
							   [](const Document& in, Document& out, const Options& /*opt*/)->Result{
			if (in.type != Document::Type::Toml) return {false,"in is not toml"};
			const auto& v = std::get<Document::TomlDom>(in.dom);
			if (!v.is_table()) return {false, "toml root is not table"};
			YAML::Node y = TomlValueToYaml(v);
			out.type = Document::Type::Yaml; out.dom = std::move(y); return {};
		});

		// YAML -> TOML
		Converter::instance().registerNativeBridge(Format::Yaml, Format::Toml,
							   [](const Document& in, Document& out, const Options& /*opt*/)->Result{
			if (in.type != Document::Type::Yaml) return {false,"in is not yaml"};
			const auto& y = std::get<Document::YamlDom>(in.dom);
			if (!y.IsMap()) return {false, "yaml root is not map"};
			toml::ordered_value v = YamlNodeToToml(y);
			if (!v.is_table()) return {false, "converted toml is not table"};
			out.type = Document::Type::Toml; out.dom = std::move(v); return {};
		});
	}
} _direct;

}} // namespace ofx::configbridge

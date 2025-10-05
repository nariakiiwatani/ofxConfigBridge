#include "ofxConfigBridgeConverter.hpp"
#include <yaml-cpp/yaml.h>
#include <nlohmann/json.hpp>
#include <string>

namespace ofx { namespace configbridge {

static bool is_integer(const std::string& s){
    if(s.empty()) return false;
    size_t i = (s[0]=='-'||s[0]=='+')?1:0;
    if(i>=s.size()) return false;
    for(; i<s.size(); ++i) if(s[i]<'0'||s[i]>'9') return false;
    return true;
}
static bool is_float(const std::string& s){
    bool dot=false, exp=false, digit=false;
    size_t i = (s[0]=='-'||s[0]=='+')?1:0;
    for(; i<s.size(); ++i){
        char c=s[i];
        if(c>='0'&&c<='9'){ digit=true; continue; }
        if(c=='.'){ if(dot) return false; dot=true; continue; }
        if(c=='e'||c=='E'){
            if(exp) return false; exp=true;
            if(i+1<s.size() && (s[i+1]=='+'||s[i+1]=='-')) ++i;
            continue;
        }
        return false;
    }
    return digit && (dot||exp);
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
    std::string s = n.as<std::string>();
    if (!opt.strict_booleans){
        if (s=="true"||s=="True") return true;
        if (s=="false"||s=="False") return false;
        if (s=="yes"||s=="on") return true;
        if (s=="no" ||s=="off") return false;
    } else {
        if (s=="true") return true;
        if (s=="false") return false;
    }
    if (!opt.strict_numbers){
        if (is_integer(s)) { try { return std::stoll(s); } catch(...){} }
        if (is_float(s))   { try { return std::stod(s); } catch(...){} }
    }
    return s;
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

	// 既存の strict_booleans / strict_numbers の処理は流用
	const std::string s = n.as<std::string>();
	// …判定して boolean / number にして返す or 文字列
	return s;
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
			YAML::Node y; JsonToYAMLNode(y, j, opt);                   // 挿入順で走査
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

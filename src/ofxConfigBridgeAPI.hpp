#pragma once
#include "ofxConfigBridgeFormat.hpp"
#include "ofxConfigBridgeRegistry.hpp"
#include "ofxConfigBridgeConverter.hpp"
#include "ofxConfigBridgeDocument.hpp"
#include <type_traits>
#include <string>

namespace ofx { namespace configbridge {

template<class Dom> struct DomFormat;


template<> struct DomFormat<nlohmann::json> {
    static constexpr Format value = Format::Json;
    static constexpr Document::Type doc_type = Document::Type::Json;
};
template<> struct DomFormat<nlohmann::ordered_json> {
    static constexpr Format value = Format::OrderedJson;
    static constexpr Document::Type doc_type = Document::Type::Json;
};
template<> struct DomFormat<YAML::Node> {
    static constexpr Format value = Format::Yaml;
    static constexpr Document::Type doc_type = Document::Type::Yaml;
};
template<> struct DomFormat<toml::ordered_value> {
    static constexpr Format value = Format::Toml;
    static constexpr Document::Type doc_type = Document::Type::Toml;
};
template<> struct DomFormat<toml::value> {
    static constexpr Format value = Format::OrderedToml;
    static constexpr Document::Type doc_type = Document::Type::Toml;
};

namespace detail {
    inline void set_doc(Document& d, nlohmann::json v){
        d.type = Document::Type::Json;
        d.dom = nlohmann::ordered_json(std::move(v));
    }
    inline void set_doc(Document& d, nlohmann::ordered_json v){ d.type = Document::Type::Json; d.dom = std::move(v); }
    inline void set_doc(Document& d, YAML::Node v){ d.type = Document::Type::Yaml;            d.dom = std::move(v); }
    inline void set_doc(Document& d, toml::ordered_value v){ d.type = Document::Type::Toml;           d.dom = std::move(v); }
    inline void set_doc(Document& d, const toml::value& v) {
        toml::ordered_value ordered_v = v;
        d.type = Document::Type::Toml;
        d.dom = std::move(ordered_v);
    }

    inline const nlohmann::ordered_json& get_oj   (const Document& d){ return std::get<nlohmann::ordered_json>(d.dom); }
    inline const YAML::Node&             get_yaml (const Document& d){ return std::get<YAML::Node>(d.dom); }
    inline const toml::ordered_value&    get_toml (const Document& d){ return std::get<toml::ordered_value>(d.dom); }

    inline nlohmann::ordered_json&       get_oj   (Document& d){ return std::get<nlohmann::ordered_json>(d.dom); }
    inline YAML::Node&                   get_yaml (Document& d){ return std::get<YAML::Node>(d.dom); }
    inline toml::ordered_value&          get_toml (Document& d){ return std::get<toml::ordered_value>(d.dom); }
}

template<class Dom>
inline ofx::configbridge::Result loadFile(const std::string& path, Dom& out) {
    constexpr Format fmt = DomFormat<Dom>::value;
    auto* ad = Registry::instance().find(fmt);
    if (!ad) return {false, "no adapter for requested format"};
	auto filepath = ofToDataPath(path);
    Document doc;
    auto r = ad->loadFile(filepath, doc);
    if (!r) return r;

    if constexpr (std::is_same_v<Dom, nlohmann::json>) {
        if (doc.type == Document::Type::Json) {
            out = detail::get_oj(doc);
        } else {
            return Result{false, "loaded document is not JSON"};
        }
    } else if constexpr (std::is_same_v<Dom, nlohmann::ordered_json>) {
        if (doc.type == Document::Type::Json) {
            out = detail::get_oj(doc);
        } else {
            return Result{false, "loaded document is not JSON(ordered)"};
        }
    } else if constexpr (std::is_same_v<Dom, YAML::Node>) {
        if (doc.type != Document::Type::Yaml) return Result{false, "loaded document is not YAML"};
        out = detail::get_yaml(doc);
    } else if constexpr (std::is_same_v<Dom, toml::ordered_value>) {
        if (doc.type != Document::Type::Toml) return Result{false, "loaded document is not TOML"};
        out = detail::get_toml(doc);
    } else if constexpr (std::is_same_v<Dom, toml::value>) {
        if (doc.type != Document::Type::Toml) return Result{false, "loaded document is not TOML"};
        auto ordered_v = detail::get_toml(doc);
        out = toml::value(ordered_v);
    } else {
        static_assert(sizeof(Dom)==0, "Unsupported Dom type");
    }
    return {};
}

template<class Dom>
inline Dom loadFile(const std::string& path) {
    Dom out;
    auto r = loadFile<Dom>(path, out);
    if (!r) throw std::runtime_error(r.message);
    return out;
}

template<class Dom>
inline Result saveFile(const std::string& path, const Dom& in, const Options& opt = {}) {
    constexpr Format fmt = DomFormat<Dom>::value;
    auto* ad = Registry::instance().find(fmt);
    if (!ad) return {false, "no adapter for requested format"};

	auto filepath = ofToDataPath(path);
    Document doc;
    detail::set_doc(doc, Dom(in));
    return ad->saveFile(doc, filepath, opt);
}

template<class Dom>
inline Result dumpText(const Dom& in, std::string& outText, const Options& opt = {}) {
    constexpr Format fmt = DomFormat<Dom>::value;
    auto* ad = Registry::instance().find(fmt);
    if (!ad) return {false, "no adapter for requested format"};
    Document doc;
    detail::set_doc(doc, Dom(in));
    return ad->dumpText(doc, outText, opt);
}

template<class Dom>
inline Result parseText(const std::string& text, Dom& out) {
    constexpr Format fmt = DomFormat<Dom>::value;
    auto* ad = Registry::instance().find(fmt);
    if (!ad) return {false, "no adapter for requested format"};
    Document doc;
    auto r = ad->parseText(text, doc);
    if (!r) return r;
    if constexpr (std::is_same_v<Dom, nlohmann::json> || std::is_same_v<Dom, nlohmann::ordered_json>) {
		if (doc.type != Document::Type::Json) return Result{false, "parsed doc is not JSON"};
		out = detail::get_oj(doc);
    } else if constexpr (std::is_same_v<Dom, YAML::Node>) {
        if (doc.type != Document::Type::Yaml) return Result{false, "parsed doc is not YAML"};
        out = detail::get_yaml(doc);
    } else if constexpr (std::is_same_v<Dom, toml::ordered_value>) {
        if (doc.type != Document::Type::Toml) return Result{false, "parsed doc is not TOML"};
        out = detail::get_toml(doc);
    } else if constexpr (std::is_same_v<Dom, toml::value>) {
        if (doc.type != Document::Type::Toml) return Result{false, "parsed doc is not TOML"};
        auto ordered_v = detail::get_toml(doc);
        out = toml::value(ordered_v);
    }
    return {};
}

template<class To, class From>
inline Result convert(const From& in, To& out, const Options& opt = {}) {
    constexpr Format toFmt   = DomFormat<To>::value;
    Document inDoc, outDoc;
    detail::set_doc(inDoc, From(in));
    auto res = Converter::instance().convert(inDoc, toFmt, outDoc, opt);
    if (!res) return res;

    if constexpr (std::is_same_v<To, nlohmann::json> || std::is_same_v<To, nlohmann::ordered_json>) {
		if (outDoc.type != Document::Type::Json) return Result{false, "converted doc is not JSON"};
		out = detail::get_oj(outDoc);
    } else if constexpr (std::is_same_v<To, YAML::Node>) {
        if (outDoc.type != Document::Type::Yaml) return Result{false, "converted doc is not YAML"};
        out = detail::get_yaml(outDoc);
    } else if constexpr (std::is_same_v<To, toml::ordered_value>) {
        if (outDoc.type != Document::Type::Toml) return Result{false, "converted doc is not TOML"};
        out = detail::get_toml(outDoc);
    } else if constexpr (std::is_same_v<To, toml::value>) {
        if (outDoc.type != Document::Type::Toml) return Result{false, "converted doc is not TOML"};
        auto ordered_v = detail::get_toml(outDoc);
        out = toml::value(ordered_v);
    }
    return {};
}

template<class To, class From>
inline To convert(const From& in, const Options& opt = {}) {
    To out{};
    auto r = convert<To>(in, out, opt);
    if (!r) throw std::runtime_error(r.message);
    return out;
}

}} // namespace ofx::configbridge

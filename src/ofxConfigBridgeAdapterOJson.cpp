#include "ofxConfigBridgeAdapterOJson.hpp"
#include "ofxConfigBridgeRegistry.hpp"
#include <fstream>
#include <sstream>

namespace ofx { namespace configbridge {
Result AdapterOJson::loadFile(const std::string& path, Document& out, const Options& opt){
  std::ifstream ifs(path, std::ios::binary); if(!ifs) return {false,"open json failed"};
  std::ostringstream oss; oss<<ifs.rdbuf(); return parseText(oss.str(), out, opt);
}
Result AdapterOJson::saveFile(const Document& in, const std::string& path, const Options& opt){
  std::string s; auto r = dumpText(in, s, opt); if(!r) return r;
  std::ofstream ofs(path, std::ios::binary); if(!ofs) return {false,"open json for write failed"};
  ofs.write(s.data(), (std::streamsize)s.size()); return {};
}
static struct _OJsonAutoReg {
  _OJsonAutoReg(){ Registry::instance().registerAdapter(std::make_unique<AdapterOJson>()); }
} _ojson_autoreg;
}} // namespace

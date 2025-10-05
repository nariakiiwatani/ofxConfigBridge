#include "ofxConfigBridgeRegistry.hpp"
#include <string>

namespace ofx { namespace configbridge {

Registry& Registry::instance(){ static Registry r; return r; }

void Registry::registerAdapter(std::unique_ptr<IAdapter> a){
    adapters_.push_back(std::move(a));
}
IAdapter* Registry::find(Format f){
    for (auto& a : adapters_) if (a->format()==f) return a.get();
    return nullptr;
}
IAdapter* Registry::findByName(const char* name){
    for (auto& a : adapters_) if (std::string(a->name())==name) return a.get();
    return nullptr;
}

}} // namespace ofx::configbridge

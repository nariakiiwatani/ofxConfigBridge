#pragma once
#include "ofxConfigBridgeAdapter.hpp"
#include <memory>
#include <vector>

namespace ofx { namespace configbridge {

class Registry {
public:
    static Registry& instance();

    void registerAdapter(std::unique_ptr<IAdapter> a);
    IAdapter* find(Format f);
    IAdapter* findByName(const char* name);

private:
    Registry() = default;
    std::vector<std::unique_ptr<IAdapter>> adapters_;
};

}} // namespace ofx::configbridge

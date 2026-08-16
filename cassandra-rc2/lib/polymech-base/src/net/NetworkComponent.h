#ifndef NET_NCOMPONENT_H
#define NET_NCOMPONENT_H

#include "Component.h"
#include <vector>
#include "NetworkValue.h"

// The template parameter N (number of NetworkValues) is kept for consistency.
template <size_t N = 20>
class NComponent : public Component {
protected:
    std::vector<NetworkValueBase *> _networkValues;
    NetworkValue<bool> m_enabled;
    uint8_t netCaps = 0;

public:
    template <typename... Args>
    NComponent(Args &&... args)
        : Component(std::forward<Args>(args)...),
          m_enabled(this, this->id, "Enabled", true, false, NetworkValue_ThresholdMode::DIFFERENCE)
    {
        _networkValues.reserve(N);
        addNetworkValue(&m_enabled);
    }

    virtual ~NComponent() = default;

    short setup() override {
        Component::setup();
        return E_OK;
    }

    short loop() override {
        Component::loop();
        if (!this->enabled()) {
            return E_OK;
        }
        return loopNetwork();
    }

    virtual short loopNetwork() { return E_OK; }

    void addNetworkValue(NetworkValueBase *nv) {
        if (nv) {
            _networkValues.push_back(nv);
        }
    }

    void addNetCapability(OBJECT_NET_CAPS cap) {
        netCaps |= (1 << cap);
    }

    bool hasNetCapability(OBJECT_NET_CAPS cap) const {
        return (netCaps & (1 << cap)) != 0;
    }

    short onMessage(int id, E_CALLS verb, E_MessageFlags flags, void* user, Component *src) override {
        if (verb == E_CALLS::EC_PROTOBUF_UPDATE && user != nullptr) {
            return this->owner->onMessage(id, verb, flags, user, src);
        }
        return Component::onMessage(id, verb, flags, user, src);
    }
};

#endif // NET_NCOMPONENT_H 
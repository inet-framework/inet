//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/common/StationLabelCache.h"

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {
namespace ieee80211 {

const std::string& StationLabelCache::getLabel(const MacAddress& receiver)
{
    auto cached = labels.find(receiver);
    if (cached != labels.end())
        return cached->second;
    // resolve the receiver MAC to its network node name (interface-table sweep); fall back to the MAC
    std::string label = receiver.str();
    L3AddressResolver resolver;
    for (cModule::SubmoduleIterator it(getSimulation()->getSystemModule()); !it.end(); ++it) {
        cModule *node = *it;
        if (!isNetworkNode(node))
            continue;
        auto interfaceTable = resolver.findInterfaceTableOf(node);
        if (interfaceTable == nullptr)
            continue;
        bool found = false;
        for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
            if (interfaceTable->getInterface(i)->getMacAddress() == receiver) {
                label = node->getFullName();
                found = true;
                break;
            }
        }
        if (found)
            break;
    }
    return labels[receiver] = label;
}

} // namespace ieee80211
} // namespace inet


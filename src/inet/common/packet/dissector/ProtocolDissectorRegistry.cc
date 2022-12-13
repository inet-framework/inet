//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"

namespace inet {

ProtocolDissectorRegistry::~ProtocolDissectorRegistry()
{
    for (auto it : protocolDissectors)
        delete it.second;
}

void ProtocolDissectorRegistry::registerProtocolDissector(const Protocol *protocol, const ProtocolDissector *dissector)
{
    protocolDissectors[protocol] = dissector;
}

const ProtocolDissector *ProtocolDissectorRegistry::findProtocolDissector(const Protocol *protocol) const
{
    auto it = protocolDissectors.find(protocol);
    return (it != protocolDissectors.end()) ? it->second : nullptr;
}

const ProtocolDissector *ProtocolDissectorRegistry::getProtocolDissector(const Protocol *protocol) const
{
    auto it = protocolDissectors.find(protocol);
    if (it != protocolDissectors.end())
        return it->second;
    else
        throw cRuntimeError("Cannot find protocol dissector for %s", protocol->getName());
}

ProtocolDissectorRegistry& ProtocolDissectorRegistry::getInstance()
{
    static int handle = cSimulationOrSharedDataManager::registerSharedVariableName("inet::ProtocolDissectorRegistry::instance");
    return getSimulationOrSharedDataManager()->getSharedVariable<ProtocolDissectorRegistry>(handle);
}

} // namespace


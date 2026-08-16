//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/common/packet/recorder/PcapCaptureAdapterRegistry.h"

namespace inet {

PcapCaptureAdapterRegistry::~PcapCaptureAdapterRegistry()
{
    for (auto entry : protocolAdapters)
        delete entry.second;
    for (auto entry : observationAdapters)
        delete entry.second;
}

void PcapCaptureAdapterRegistry::registerProtocolAdapter(const Protocol *protocol, const IPcapCaptureAdapter *adapter)
{
    if (protocol == nullptr || adapter == nullptr || protocolAdapters.find(protocol) != protocolAdapters.end()) {
        delete adapter;
        throw cRuntimeError("Duplicate or invalid PCAP capture protocol adapter registration");
    }
    protocolAdapters.emplace(protocol, adapter);
}

void PcapCaptureAdapterRegistry::registerProtocolResolver(const Protocol *outerProtocol, const Protocol *captureProtocol)
{
    if (outerProtocol == nullptr || captureProtocol == nullptr || protocolResolvers.find(outerProtocol) != protocolResolvers.end())
        throw cRuntimeError("Duplicate or invalid PCAP capture protocol resolver registration");
    protocolResolvers.emplace(outerProtocol, captureProtocol);
}

void PcapCaptureAdapterRegistry::registerObservationAdapter(const char *key, const IPcapCaptureObservationAdapter *adapter)
{
    if (opp_isempty(key) || adapter == nullptr || observationAdapters.find(key) != observationAdapters.end()) {
        delete adapter;
        throw cRuntimeError("Duplicate or invalid PCAP capture observation adapter registration for '%s'", key == nullptr ? "" : key);
    }
    observationAdapters.emplace(key, adapter);
}

const IPcapCaptureAdapter *PcapCaptureAdapterRegistry::findProtocolAdapter(const Protocol *protocol) const
{
    auto iterator = protocolAdapters.find(protocol);
    return iterator == protocolAdapters.end() ? nullptr : iterator->second;
}

std::optional<std::tuple<const Protocol *, b, b>> PcapCaptureAdapterRegistry::tryResolveProtocol(const Protocol *outerProtocol, const Packet *packet, b frontOffset, b backOffset) const
{
    auto resolver = protocolResolvers.find(outerProtocol);
    if (resolver == protocolResolvers.end())
        return std::nullopt;
    auto adapter = findProtocolAdapter(resolver->second);
    if (adapter == nullptr)
        return std::nullopt;
    auto offsets = adapter->tryResolvePacket(packet, frontOffset, backOffset);
    return offsets.has_value() ? std::optional<std::tuple<const Protocol *, b, b>>({resolver->second, offsets->first, offsets->second}) : std::nullopt;
}

std::optional<PcapCaptureObservation> PcapCaptureAdapterRegistry::tryCreateObservation(const cObject *object, Direction direction) const
{
    for (const auto& entry : observationAdapters) {
        auto observation = entry.second->tryCreateObservation(object, direction);
        if (observation.has_value())
            return observation;
    }
    return std::nullopt;
}

PcapCaptureAdapterRegistry& PcapCaptureAdapterRegistry::getInstance()
{
    static int handle = cSimulationOrSharedDataManager::registerSharedVariableName("inet::PcapCaptureAdapterRegistry::instance");
    return getSimulationOrSharedDataManager()->getSharedVariable<PcapCaptureAdapterRegistry>(handle);
}

} // namespace inet

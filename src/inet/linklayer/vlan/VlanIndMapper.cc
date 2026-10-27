//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/vlan/VlanIndMapper.h"

#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

Define_Module(VlanIndMapper);

static const cValueMap *getVlanIdMap(const cComponent *component, const std::string& interfaceName, const cValue& value)
{
    auto map = value.getType() == cValue::POINTER ? dynamic_cast<const cValueMap *>(value.objectValue()) : nullptr;
    if (map == nullptr)
        throw cRuntimeError(component, "Invalid mappedVlanIds entry for interface '%s': expected a map of VLAN ids, got %s", interfaceName.c_str(), value.str().c_str());
    return map;
}

static int parseVlanIdKey(const cComponent *component, const std::string& interfaceName, const std::string& key)
{
    char *end = nullptr;
    long vlanId = strtol(key.c_str(), &end, 10);
    if (key.empty() || *end != '\0' || vlanId < -1 || vlanId > 4094 || std::to_string(vlanId) != key)
        throw cRuntimeError(component, "Invalid VLAN id key '%s' in the mappedVlanIds entry for interface '%s': expected the decimal form of -1 or 0 to 4094", key.c_str(), interfaceName.c_str());
    return vlanId;
}

static int parseMappedVlanId(const cComponent *component, const std::string& interfaceName, const cValue& value)
{
    if (value.getType() != cValue::INT || (value.intValue() != -1 && (value.intValue() < 1 || value.intValue() > 4094)))
        throw cRuntimeError(component, "Invalid VLAN id %s in the mappedVlanIds entry for interface '%s': expected an integer, -1 or 1 to 4094", value.str().c_str(), interfaceName.c_str());
    return value.intValue();
}

void VlanIndMapper::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        interfaceTable.reference(this, "interfaceTableModule", true);
        auto map = check_and_cast<cValueMap *>(par("mappedVlanIds").objectValue());
        for (const auto& [interfaceName, value] : map->getFields()) {
            auto& interfaceMappedVlanIds = mappedVlanIds[interfaceName];
            for (const auto& [key, mappedVlanId] : getVlanIdMap(this, interfaceName, value)->getFields()) {
                int vlanId = parseVlanIdKey(this, interfaceName, key);
                interfaceMappedVlanIds[vlanId] = parseMappedVlanId(this, interfaceName, mappedVlanId);
            }
        }
    }
}

cGate *VlanIndMapper::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

void VlanIndMapper::processPacket(Packet *packet)
{
    auto interfaceInd = packet->findTag<InterfaceInd>();
    auto networkInterface = interfaceInd != nullptr ? interfaceTable->getInterfaceById(interfaceInd->getInterfaceId()) : nullptr;
    std::string interfaceName = networkInterface != nullptr ? networkInterface->getInterfaceName() : "";
    auto it = mappedVlanIds.find(interfaceName);
    if (it == mappedVlanIds.end())
        it = mappedVlanIds.find("*");
    if (it != mappedVlanIds.end()) {
        auto vlanInd = packet->findTag<VlanInd>();
        auto receivedVlanId = vlanInd != nullptr ? vlanInd->getVlanId() : -1;
        auto jt = it->second.find(receivedVlanId);
        if (jt != it->second.end()) {
            auto mappedVlanId = jt->second;
            EV_INFO << "Mapping VLAN id" << EV_FIELD(interfaceName) << EV_FIELD(receivedVlanId) << EV_FIELD(mappedVlanId) << EV_ENDL;
            if (mappedVlanId == -1)
                packet->removeTagIfPresent<VlanInd>();
            else
                packet->addTagIfAbsent<VlanInd>()->setVlanId(mappedVlanId);
        }
    }
}

} // namespace inet

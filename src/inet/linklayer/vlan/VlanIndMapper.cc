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

void VlanIndMapper::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        interfaceTable.reference(this, "interfaceTableModule", true);
        mappedVlanIds = parseMappedVlanIds(this, check_and_cast<cValueMap *>(par("mappedVlanIds").objectValue()), 1);
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
    if (auto interfaceMappedVlanIds = findInterfaceEntry(mappedVlanIds, interfaceName)) {
        auto vlanInd = packet->findTag<VlanInd>();
        auto receivedVlanId = vlanInd != nullptr ? vlanInd->getVlanId() : -1;
        auto it = interfaceMappedVlanIds->find(receivedVlanId);
        if (it != interfaceMappedVlanIds->end()) {
            auto mappedVlanId = it->second;
            EV_INFO << "Mapping VLAN id" << EV_FIELD(interfaceName) << EV_FIELD(receivedVlanId) << EV_FIELD(mappedVlanId) << EV_ENDL;
            if (mappedVlanId == -1)
                packet->removeTagIfPresent<VlanInd>();
            else
                packet->addTagIfAbsent<VlanInd>()->setVlanId(mappedVlanId);
        }
    }
}

} // namespace inet

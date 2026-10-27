//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/vlan/VlanReqMapper.h"

#include "inet/common/ProtocolUtils.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/PcpTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

Define_Module(VlanReqMapper);

void VlanReqMapper::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        protocol = Protocol::getProtocol(par("protocol").stringValue());
        interfaceTable.reference(this, "interfaceTableModule", true);
        mappedVlanIds = parseMappedVlanIds(this, check_and_cast<cValueMap *>(par("mappedVlanIds").objectValue()), 0);
    }
}

cGate *VlanReqMapper::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

void VlanReqMapper::processPacket(Packet *packet)
{
    auto interfaceReq = packet->findTag<InterfaceReq>();
    auto networkInterface = interfaceReq != nullptr ? interfaceTable->getInterfaceById(interfaceReq->getInterfaceId()) : nullptr;
    std::string interfaceName = networkInterface != nullptr ? networkInterface->getInterfaceName() : "";
    if (auto interfaceMappedVlanIds = findInterfaceEntry(mappedVlanIds, interfaceName)) {
        auto vlanReq = packet->findTag<VlanReq>();
        auto requestedVlanId = vlanReq != nullptr ? vlanReq->getVlanId() : -1;
        auto it = interfaceMappedVlanIds->find(requestedVlanId);
        if (it != interfaceMappedVlanIds->end()) {
            auto mappedVlanId = it->second;
            if (mappedVlanId == -1)
                packet->removeTagIfPresent<VlanReq>();
            else
                packet->addTagIfAbsent<VlanReq>()->setVlanId(mappedVlanId);
        }
    }
    ensureEncapsulationProtocolReq(packet, protocol, packet->hasTag<VlanReq>() || packet->hasTag<PcpReq>());
    setDispatchProtocol(packet);
}

} // namespace inet


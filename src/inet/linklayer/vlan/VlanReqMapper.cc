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
        cObject *object = par("mappedVlanIds");
        mappedVlanIds = check_and_cast<cValueMap *>(object);
        WATCH(mappedVlanIds);
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
    auto interfaceName = networkInterface != nullptr ? networkInterface->getInterfaceName() : "";
    auto key = mappedVlanIds->containsKey(interfaceName) ? interfaceName : "*";
    if (mappedVlanIds->containsKey(key)) {
        auto interfaceMappedVlanIds = check_and_cast<cValueMap *>(mappedVlanIds->get(key).objectValue());
        auto vlanReq = packet->findTag<VlanReq>();
        auto oldVlanId = vlanReq != nullptr ? vlanReq->getVlanId() : -1;
        auto key = std::to_string(oldVlanId);
        if (interfaceMappedVlanIds->containsKey(key.c_str())) {
            auto newVlanId = interfaceMappedVlanIds->get(key.c_str()).intValue();
            if (newVlanId == -1)
                packet->removeTagIfPresent<VlanReq>();
            else
                packet->addTagIfAbsent<VlanReq>()->setVlanId(newVlanId);
        }
    }
    ensureEncapsulationProtocolReq(packet, protocol, packet->hasTag<VlanReq>() || packet->hasTag<PcpReq>());
    setDispatchProtocol(packet);
}

} // namespace inet


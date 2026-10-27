//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/vlan/VlanReqFilter.h"

#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

Define_Module(VlanReqFilter);

void VlanReqFilter::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        interfaceTable.reference(this, "interfaceTableModule", true);
        acceptedVlanIds = parseAcceptedVlanIds(this, check_and_cast<cValueMap *>(par("acceptedVlanIds").objectValue()));
    }
}

cGate *VlanReqFilter::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

void VlanReqFilter::dropPacket(Packet *packet)
{
    EV_WARN << "Received packet is not accepted, dropping packet" << EV_FIELD(packet) << EV_ENDL;
    PacketFilterBase::dropPacket(packet, OTHER_PACKET_DROP);
}

bool VlanReqFilter::matchesPacket(const Packet *packet) const
{
    auto interfaceReq = packet->findTag<InterfaceReq>();
    auto networkInterface = interfaceReq != nullptr ? interfaceTable->getInterfaceById(interfaceReq->getInterfaceId()) : nullptr;
    std::string interfaceName = networkInterface != nullptr ? networkInterface->getInterfaceName() : "";
    auto interfaceVlanIds = findInterfaceEntry(acceptedVlanIds, interfaceName);
    if (interfaceVlanIds == nullptr)
        return false;
    auto vlanReq = packet->findTag<VlanReq>();
    auto vlanId = vlanReq != nullptr ? vlanReq->getVlanId() : -1;
    return interfaceVlanIds->count(vlanId) != 0;
}

} // namespace inet


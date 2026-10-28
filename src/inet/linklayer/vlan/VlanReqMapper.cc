//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/vlan/VlanReqMapper.h"

#include "inet/common/ProtocolUtils.h"
#include "inet/common/Simsignals.h"
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
        vlanTagType = parseVlanTagType(par("vlanTagType"));
        protocol = getVlanTagProtocol(vlanTagType);
        interfaceTable.reference(this, "interfaceTableModule", true);
        mappedVlanIds = parseMappedVlanIds(this, check_and_cast<cValueMap *>(par("mappedVlanIds").objectValue()), 0);
        WATCH(mappedVlanIds);
        check_and_cast<cModule *>(interfaceTable.get())->subscribe(interfaceDeletedSignal, this);
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
        auto vlanReq = findVlanReq(packet, vlanTagType);
        auto requestedVlanId = vlanReq != nullptr ? vlanReq->getVlanId() : -1;
        auto it = interfaceMappedVlanIds->find(requestedVlanId);
        if (it != interfaceMappedVlanIds->end()) {
            auto mappedVlanId = it->second;
            if (mappedVlanId == -1)
                removeVlanReqIfPresent(packet, vlanTagType);
            else
                addVlanReqIfAbsent(packet, vlanTagType)->setVlanId(mappedVlanId);
        }
    }
    ensureEncapsulationProtocolReq(packet, protocol, findVlanReq(packet, vlanTagType) != nullptr || findPcpReq(packet, vlanTagType) != nullptr);
    setDispatchProtocol(packet);
}

void VlanReqMapper::setInterfaceVlanIdMap(const NetworkInterface *networkInterface, const std::map<int, int>& vlanIdMap)
{
    Enter_Method("setInterfaceVlanIdMap");
    auto interfaceName = networkInterface->getInterfaceName();
    if (interfaceTable->getInterfaceById(networkInterface->getInterfaceId()) != networkInterface)
        throw cRuntimeError("Interface '%s' is not in the interface table of this module", interfaceName);
    checkMappedVlanIds(this, interfaceName, vlanIdMap, 0);
    EV_INFO << "Setting VLAN id map" << EV_FIELD(interfaceName) << EV_ENDL;
    mappedVlanIds[interfaceName] = vlanIdMap;
}

void VlanReqMapper::removeInterfaceVlanIdMap(const NetworkInterface *networkInterface)
{
    Enter_Method("removeInterfaceVlanIdMap");
    auto interfaceName = networkInterface->getInterfaceName();
    EV_INFO << "Removing VLAN id map" << EV_FIELD(interfaceName) << EV_ENDL;
    mappedVlanIds.erase(interfaceName);
}

void VlanReqMapper::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));
    if (signal == interfaceDeletedSignal)
        mappedVlanIds.erase(check_and_cast<NetworkInterface *>(object)->getInterfaceName());
    else
        throw cRuntimeError("Unknown signal");
}

} // namespace inet


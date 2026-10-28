//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/vlan/VlanIndMapper.h"

#include "inet/common/Simsignals.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

Define_Module(VlanIndMapper);

void VlanIndMapper::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        vlanTagType = parseVlanTagType(par("vlanTagType"));
        interfaceTable.reference(this, "interfaceTableModule", true);
        mappedVlanIds = parseMappedVlanIds(this, check_and_cast<cValueMap *>(par("mappedVlanIds").objectValue()), 1);
        WATCH(mappedVlanIds);
        check_and_cast<cModule *>(interfaceTable.get())->subscribe(interfaceDeletedSignal, this);
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
        auto vlanInd = findVlanInd(packet, vlanTagType);
        auto receivedVlanId = vlanInd != nullptr ? vlanInd->getVlanId() : -1;
        auto it = interfaceMappedVlanIds->find(receivedVlanId);
        if (it != interfaceMappedVlanIds->end()) {
            auto mappedVlanId = it->second;
            EV_INFO << "Mapping VLAN id" << EV_FIELD(interfaceName) << EV_FIELD(receivedVlanId) << EV_FIELD(mappedVlanId) << EV_ENDL;
            if (mappedVlanId == -1)
                removeVlanIndIfPresent(packet, vlanTagType);
            else
                addVlanIndIfAbsent(packet, vlanTagType)->setVlanId(mappedVlanId);
        }
    }
}

void VlanIndMapper::setInterfaceVlanIdMap(const NetworkInterface *networkInterface, const std::map<int, int>& vlanIdMap)
{
    Enter_Method("setInterfaceVlanIdMap");
    auto interfaceName = networkInterface->getInterfaceName();
    if (interfaceTable->getInterfaceById(networkInterface->getInterfaceId()) != networkInterface)
        throw cRuntimeError("Interface '%s' is not in the interface table of this module", interfaceName);
    checkMappedVlanIds(this, interfaceName, vlanIdMap, 1);
    EV_INFO << "Setting VLAN id map" << EV_FIELD(interfaceName) << EV_ENDL;
    mappedVlanIds[interfaceName] = vlanIdMap;
}

void VlanIndMapper::removeInterfaceVlanIdMap(const NetworkInterface *networkInterface)
{
    Enter_Method("removeInterfaceVlanIdMap");
    auto interfaceName = networkInterface->getInterfaceName();
    EV_INFO << "Removing VLAN id map" << EV_FIELD(interfaceName) << EV_ENDL;
    mappedVlanIds.erase(interfaceName);
}

void VlanIndMapper::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));
    if (signal == interfaceDeletedSignal)
        mappedVlanIds.erase(check_and_cast<NetworkInterface *>(object)->getInterfaceName());
    else
        throw cRuntimeError("Unknown signal");
}

} // namespace inet

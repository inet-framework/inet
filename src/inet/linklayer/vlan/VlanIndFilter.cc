//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/vlan/VlanIndFilter.h"

#include "inet/common/Simsignals.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

Define_Module(VlanIndFilter);

void VlanIndFilter::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        interfaceTable.reference(this, "interfaceTableModule", true);
        acceptedVlanIds = parseAcceptedVlanIds(this, check_and_cast<cValueMap *>(par("acceptedVlanIds").objectValue()));
        WATCH(acceptedVlanIds);
        check_and_cast<cModule *>(interfaceTable.get())->subscribe(interfaceDeletedSignal, this);
    }
}

cGate *VlanIndFilter::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

void VlanIndFilter::dropPacket(Packet *packet)
{
    EV_WARN << "Received packet is not accepted, dropping packet" << EV_FIELD(packet) << EV_ENDL;
    PacketFilterBase::dropPacket(packet, OTHER_PACKET_DROP);
}

bool VlanIndFilter::matchesPacket(const Packet *packet) const
{
    auto interfaceInd = packet->findTag<InterfaceInd>();
    auto networkInterface = interfaceInd != nullptr ? interfaceTable->getInterfaceById(interfaceInd->getInterfaceId()) : nullptr;
    std::string interfaceName = networkInterface != nullptr ? networkInterface->getInterfaceName() : "";
    auto interfaceVlanIds = findInterfaceEntry(acceptedVlanIds, interfaceName);
    if (interfaceVlanIds == nullptr)
        return false;
    auto vlanInd = packet->findTag<CVlanInd>();
    auto vlanId = vlanInd != nullptr ? vlanInd->getVlanId() : -1;
    return interfaceVlanIds->count(vlanId) != 0;
}

void VlanIndFilter::setInterfaceAcceptedVlanIds(const NetworkInterface *networkInterface, const std::set<int>& vlanIds)
{
    Enter_Method("setInterfaceAcceptedVlanIds");
    auto interfaceName = networkInterface->getInterfaceName();
    if (interfaceTable->getInterfaceById(networkInterface->getInterfaceId()) != networkInterface)
        throw cRuntimeError("Interface '%s' is not in the interface table of this module", interfaceName);
    checkAcceptedVlanIds(this, interfaceName, vlanIds);
    EV_INFO << "Setting accepted VLAN ids" << EV_FIELD(interfaceName) << EV_ENDL;
    acceptedVlanIds[interfaceName] = vlanIds;
}

void VlanIndFilter::removeInterfaceAcceptedVlanIds(const NetworkInterface *networkInterface)
{
    Enter_Method("removeInterfaceAcceptedVlanIds");
    auto interfaceName = networkInterface->getInterfaceName();
    EV_INFO << "Removing accepted VLAN ids" << EV_FIELD(interfaceName) << EV_ENDL;
    acceptedVlanIds.erase(interfaceName);
}

void VlanIndFilter::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));
    if (signal == interfaceDeletedSignal)
        acceptedVlanIds.erase(check_and_cast<NetworkInterface *>(object)->getInterfaceName());
    else
        throw cRuntimeError("Unknown signal");
}

} // namespace inet


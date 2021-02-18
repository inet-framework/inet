//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
        interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        cObject *object = par("mappedVlanIds");
        mappedVlanIds = check_and_cast<cValueMap *>(object);
        WATCH(mappedVlanIds);
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
    auto interfaceName = networkInterface != nullptr ? networkInterface->getInterfaceName() : "";
    auto key = mappedVlanIds->containsKey(interfaceName) ? interfaceName : "*";
    if (mappedVlanIds->containsKey(key)) {
        auto interfaceMappedVlanIds = check_and_cast<cValueMap *>(mappedVlanIds->get(key).objectValue());
        auto vlanInd = packet->findTag<VlanInd>();
        auto oldVlanId = vlanInd != nullptr ? vlanInd->getVlanId() : -1;
        auto key = std::to_string(oldVlanId);
        if (interfaceMappedVlanIds->containsKey(key.c_str())) {
            auto newVlanId = interfaceMappedVlanIds->get(key.c_str()).intValue();
            if (newVlanId == -1)
                packet->removeTagIfPresent<VlanInd>();
            else
                packet->addTagIfAbsent<VlanInd>()->setVlanId(newVlanId);
        }
    }
    auto dispatchProtocolInd = packet->findTag<DispatchProtocolInd>();
    if (dispatchProtocolInd != nullptr)
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(dispatchProtocolInd->getProtocol());
}

} // namespace inet


//
// Copyright (C) OpenSim Ltd.
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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ieee8021q/Ieee8021QTag_m.h"
#include "inet/linklayer/vlan/VlanInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

Define_Module(VlanInterface);
Define_Module(Vlan);

void VlanInterface::initialize(int stage)
{
    InterfaceEntry::initialize(stage);
    if (stage == INITSTAGE_NETWORK_INTERFACE_CONFIGURATION) {
        const char *addressString = par("address");
        MacAddress address = strcmp(addressString, "auto") ? MacAddress(addressString) : MacAddress::generateAutoAddress();
        setMacAddress(address);
        setInterfaceToken(address.formInterfaceIdentifier());
        setBroadcast(true);      //TODO
        setMulticast(true);      //TODO
        setPointToPoint(true);   //TODO
    }
}

void Vlan::initialize(int stage)
{
    if (stage == INITSTAGE_LINK_LAYER) {
        vlanId = par("vlanId");
        auto interfaceTable = findModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        interfaceEntry = interfaceTable->getInterfaceByName(par("interfaceName"));
    }
}

void Vlan::handleMessage(cMessage *message)
{
    auto packet = check_and_cast<Packet *>(message);
    packet->addTagIfAbsent<Ieee8021QReq>()->setVid(vlanId);
    packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(interfaceEntry->getInterfaceId());
    send(message, "upperLayerOut");
}

} // namespace inet


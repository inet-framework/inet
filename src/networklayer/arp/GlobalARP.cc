/*
 * Copyright (C) 2004 Andras Varga
 * Copyright (C) 2008 Alfonso Ariza Quintana (global arp)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#include "GlobalARP.h"
#include "InterfaceEntry.h"
#include "InterfaceTableAccess.h"
#include "GenericNetworkProtocolControlInfo_m.h"
#include "Ieee802Ctrl_m.h"

Define_Module(GlobalARP);


void GlobalARP::initialize(int stage)
{
    InetSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        ift = InterfaceTableAccess().get();
        nicOutBaseGateId = gateSize("nicOut")==0 ? -1 : gate("nicOut", 0)->getId();
    }
}

void GlobalARP::handleMessage(cMessage *msg)
{
    GenericRoutingDecision *controlInfo = check_and_cast<GenericRoutingDecision*>(msg->removeControlInfo());
    Address nextHop = controlInfo->getNextHop();
    InterfaceEntry *ie = ift->getInterfaceById(controlInfo->getInterfaceId());
    delete controlInfo;

    // if output interface is not broadcast, don't bother with ARP
    if (!ie->isBroadcast())
    {
        EV << "output interface " << ie->getName() << " is not broadcast, skipping ARP\n";
        sendSync(msg, nicOutBaseGateId + ie->getNetworkLayerGateIndex());
        return;
    }

    // determine what address to look up in ARP cache
    if (nextHop.isUnspecified())
        throw cRuntimeError("No next hop");

    if (nextHop.isMulticast())
    {
        MACAddress macAddr = mapMulticastAddress(nextHop);
        EV << "destination address is multicast, sending packet to MAC address " << macAddr << "\n";
        sendPacketToNIC(msg, ie, macAddr, ETHERTYPE_IPv4); // TODO:
        return;
    }

    // valid ARP cache entry found, flag msg with MAC address and send it out
    sendPacketToNIC(msg, ie, mapUnicastAddress(nextHop), ETHERTYPE_IPv4); // TODO:
}

MACAddress GlobalARP::mapUnicastAddress(Address addr)
{
    cModule * module;
    switch (addr.getType()) {
        case Address::MAC:
            return addr.toMAC();
        case Address::MODULEID:
            module = simulation.getModule(addr.toModuleId().getId());
            break;
        case Address::MODULEPATH:
            module = simulation.getModule(addr.toModulePath().getId());
            break;
        default:
            throw cRuntimeError("Unknown address type");
    }
    IInterfaceTable * interfaceTable = InterfaceTableAccess().get(module);
    InterfaceEntry * interfaceEntry = interfaceTable->getInterfaceByInterfaceModule(module);
    return interfaceEntry->getMacAddress();
}

MACAddress GlobalARP::mapMulticastAddress(Address addr)
{
    ASSERT(addr.isMulticast());

    MACAddress macAddr;
    macAddr.setAddressByte(0, 0x01);
    macAddr.setAddressByte(1, 0x00);
    macAddr.setAddressByte(2, 0x5e);
    // TODO:
    // macAddr.setAddressByte(3, addr.getDByte(1) & 0x7f);
    // macAddr.setAddressByte(4, addr.getDByte(2));
    // macAddr.setAddressByte(5, addr.getDByte(3));
    return macAddr;
}

void GlobalARP::sendPacketToNIC(cMessage *msg, InterfaceEntry *ie, const MACAddress& macAddress, int etherType)
{
    // add control info with MAC address
    Ieee802Ctrl *controlInfo = new Ieee802Ctrl();
    controlInfo->setDest(macAddress);
    controlInfo->setEtherType(etherType);
    msg->setControlInfo(controlInfo);

    // send out
    sendSync(msg, nicOutBaseGateId + ie->getNetworkLayerGateIndex());
}

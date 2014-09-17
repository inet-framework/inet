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

#include "inet/networklayer/arp/generic/GenericARP.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/common/IInterfaceTable.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/contract/generic/GenericNetworkProtocolControlInfo_m.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"

namespace inet {

Define_Module(GenericARP);

MACAddress GenericARP::resolveL3Address(const L3Address& address, const InterfaceEntry *ie)
{
    if (address.isUnicast())
        return mapUnicastAddress(address);
    else if (address.isMulticast())
        return mapMulticastAddress(address);
    else if (address.isBroadcast())
        return MACAddress::BROADCAST_ADDRESS;
    throw cRuntimeError("address must be one of unicast or multicast or broadcast");
}

void GenericARP::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
    }
}

void GenericARP::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
        throw cRuntimeError("This module doesn't accept any self message");

    EV << "received a " << msg << " message, dropped\n";
    delete msg;
}

MACAddress GenericARP::mapUnicastAddress(L3Address addr)
{
    cModule *module;
    switch (addr.getType()) {
        case L3Address::MAC:
            return addr.toMAC();

        case L3Address::MODULEID:
            module = simulation.getModule(addr.toModuleId().getId());
            break;

        case L3Address::MODULEPATH:
            module = simulation.getModule(addr.toModulePath().getId());
            break;

        default:
            throw cRuntimeError("Unknown address type");
    }
    IInterfaceTable *interfaceTable = L3AddressResolver().findInterfaceTableOf(getContainingNode(module));
    InterfaceEntry *interfaceEntry = interfaceTable->getInterfaceByInterfaceModule(module);
    return interfaceEntry->getMacAddress();
}

MACAddress GenericARP::mapMulticastAddress(L3Address addr)
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

} // namespace inet


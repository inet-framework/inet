//
// Copyright (C) 2015 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "Ieee80211NewMacMacSap.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/linklayer/ieee80211/thenewmac/signals/Ieee80211MacSignals_m.h"


namespace inet {
namespace ieee80211 {

Define_Module(Ieee80211NewMacMacSap);

void Ieee80211NewMacMacSap::initialize(int stage)
{
    if (stage == INITSTAGE_LINK_LAYER) {
        address = MACAddress::generateAutoAddress();
        registerInterface();
    }
}

void Ieee80211NewMacMacSap::registerInterface()
{
    ASSERT(interfaceEntry == nullptr);
    IInterfaceTable *interfaceTable = findModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
    if (interfaceTable) {
        interfaceEntry = createInterfaceEntry();
        interfaceTable->addInterface(interfaceEntry);
    }
}

InterfaceEntry *Ieee80211NewMacMacSap::createInterfaceEntry()
{
    InterfaceEntry *e = new InterfaceEntry(this);

    // address
    e->setMACAddress(address);
    e->setInterfaceToken(address.formInterfaceIdentifier());

//    e->setMtu(par("mtu").longValue());

    // capabilities
    e->setBroadcast(true);
    e->setMulticast(true);
    e->setPointToPoint(false);

    return e;
}

void Ieee80211NewMacMacSap::handleMessage(cMessage* msg)
{
    Ieee802Ctrl *controlInfo =  check_and_cast<Ieee802Ctrl *>(msg->getControlInfo());
    Ieee80211MacSignalMaUnitDataRequest *maUnitDataReq = new Ieee80211MacSignalMaUnitDataRequest();
    maUnitDataReq->setDestinationAddress(controlInfo->getDest());
    send(maUnitDataReq, "macSap$o");
}

} /* namespace inet */
} /* namespace ieee80211 */


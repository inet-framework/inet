//
// Copyright (C) 2006 Andras Varga
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

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/EtherTypeTag_m.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MACAddressTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/ieee802/Ieee802LlcHeader_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtSTASimplified.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

namespace ieee80211 {

Define_Module(Ieee80211MgmtSTASimplified);

void Ieee80211MgmtSTASimplified::initialize(int stage)
{
    Ieee80211MgmtBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        mib->mode = Ieee80211Mib::INFRASTRUCTURE;
        mib->bssStationData.stationType = Ieee80211Mib::STATION;
        mib->bssStationData.isAssociated = true;
        mib->bssData.bssid.setAddress(par("accessPointAddress").stringValue());
    }
    else if (stage == INITSTAGE_LINK_LAYER_2) {
        L3AddressResolver addressResolver;
        auto host = addressResolver.findHostWithAddress(mib->bssData.bssid);
        auto interfaceTable = addressResolver.findInterfaceTableOf(host);
        auto interfaceEntry = interfaceTable->findInterfaceByAddress(mib->bssData.bssid);
        auto apMib = dynamic_cast<Ieee80211Mib *>(interfaceEntry->getInterfaceModule()->getSubmodule("mib"));
        apMib->bssAccessPointData.stations[mib->address] = Ieee80211Mib::ASSOCIATED;
    }
}

void Ieee80211MgmtSTASimplified::handleTimer(cMessage *msg)
{
    ASSERT(false);
}

void Ieee80211MgmtSTASimplified::handleCommand(int msgkind, cObject *ctrl)
{
    throw cRuntimeError("handleCommand(): no commands supported");
}

void Ieee80211MgmtSTASimplified::handleAuthenticationFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtSTASimplified::handleDeauthenticationFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtSTASimplified::handleAssociationRequestFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtSTASimplified::handleAssociationResponseFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtSTASimplified::handleReassociationRequestFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtSTASimplified::handleReassociationResponseFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtSTASimplified::handleDisassociationFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtSTASimplified::handleBeaconFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtSTASimplified::handleProbeRequestFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtSTASimplified::handleProbeResponseFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    dropManagementFrame(packet);
}

} // namespace ieee80211

} // namespace inet


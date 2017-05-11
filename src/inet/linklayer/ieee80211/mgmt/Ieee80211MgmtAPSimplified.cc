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

#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtAPSimplified.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"

#ifdef WITH_ETHERNET
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#endif // ifdef WITH_ETHERNET

namespace inet {

namespace ieee80211 {

Define_Module(Ieee80211MgmtAPSimplified);

// FIXME add sequence number handling

void Ieee80211MgmtAPSimplified::handleTimer(cMessage *msg)
{
    ASSERT(false);
}

void Ieee80211MgmtAPSimplified::handleCommand(int msgkind, cObject *ctrl)
{
    throw cRuntimeError("handleCommand(): no commands supported");
}

void Ieee80211MgmtAPSimplified::handleAuthenticationFrame(Packet *packet, const Ptr<Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAPSimplified::handleDeauthenticationFrame(Packet *packet, const Ptr<Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAPSimplified::handleAssociationRequestFrame(Packet *packet, const Ptr<Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAPSimplified::handleAssociationResponseFrame(Packet *packet, const Ptr<Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAPSimplified::handleReassociationRequestFrame(Packet *packet, const Ptr<Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAPSimplified::handleReassociationResponseFrame(Packet *packet, const Ptr<Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAPSimplified::handleDisassociationFrame(Packet *packet, const Ptr<Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAPSimplified::handleBeaconFrame(Packet *packet, const Ptr<Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAPSimplified::handleProbeRequestFrame(Packet *packet, const Ptr<Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAPSimplified::handleProbeResponseFrame(Packet *packet, const Ptr<Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

} // namespace ieee80211

} // namespace inet


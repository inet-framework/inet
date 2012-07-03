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


#include "Ieee80211MgmtAPSimplified.h"
#include "Ieee802Ctrl_m.h"

#ifdef WITH_ETHERNET
#include "EtherFrame_m.h"
#endif


Define_Module(Ieee80211MgmtAPSimplified);


// FIXME add sequence number handling

void Ieee80211MgmtAPSimplified::initialize(int stage)
{
    Ieee80211MgmtAPBase::initialize(stage);
}

void Ieee80211MgmtAPSimplified::handleTimer(cMessage *msg)
{
    ASSERT(false);
}

void Ieee80211MgmtAPSimplified::handleUpperMessage(cPacket *msg)
{
    Ieee80211DataFrame *frame = encapsulate(msg);
    sendOrEnqueue(frame);
}

void Ieee80211MgmtAPSimplified::handleCommand(int msgkind, cObject *ctrl)
{
    error("handleCommand(): no commands supported");
}

void Ieee80211MgmtAPSimplified::receiveChangeNotification(int category, const cObject *details)
{
    Enter_Method_Silent();
    printNotificationBanner(category, details);
}

void Ieee80211MgmtAPSimplified::handleDataFrame(Ieee80211DataFrame *frame)
{
    // check toDS bit
    if (!frame->getToDS())
    {
        // looks like this is not for us - discard
        delete frame;
        return;
    }

    if (isConnectedToHL)
        sendToUpperLayer(frame->dup());

    // send it out to the destination STA
    distributeReceivedDataFrame(frame);
}

void Ieee80211MgmtAPSimplified::handleAuthenticationFrame(Ieee80211AuthenticationFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAPSimplified::handleDeauthenticationFrame(Ieee80211DeauthenticationFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAPSimplified::handleAssociationRequestFrame(Ieee80211AssociationRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAPSimplified::handleAssociationResponseFrame(Ieee80211AssociationResponseFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAPSimplified::handleReassociationRequestFrame(Ieee80211ReassociationRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAPSimplified::handleReassociationResponseFrame(Ieee80211ReassociationResponseFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAPSimplified::handleDisassociationFrame(Ieee80211DisassociationFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAPSimplified::handleBeaconFrame(Ieee80211BeaconFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAPSimplified::handleProbeRequestFrame(Ieee80211ProbeRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAPSimplified::handleProbeResponseFrame(Ieee80211ProbeResponseFrame *frame)
{
    dropManagementFrame(frame);
}


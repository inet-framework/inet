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


#include "Ieee80211MgmtSTASimplified.h"
#include "Ieee802Ctrl_m.h"


Define_Module(Ieee80211MgmtSTASimplified);


void Ieee80211MgmtSTASimplified::initialize(int stage)
{
    Ieee80211MgmtBase::initialize(stage);
    if (stage==0)
    {
        accessPointAddress.setAddress(par("accessPointAddress").stringValue());
        receiveSequence = 0;
    }
}

void Ieee80211MgmtSTASimplified::handleTimer(cMessage *msg)
{
    ASSERT(false);
}

void Ieee80211MgmtSTASimplified::handleUpperMessage(cPacket *msg)
{
    Ieee80211DataFrame *frame = encapsulate(msg);
    sendOrEnqueue(frame);
}

void Ieee80211MgmtSTASimplified::handleCommand(int msgkind, cPolymorphic *ctrl)
{
    error("handleCommand(): no commands supported");
}

Ieee80211DataFrame *Ieee80211MgmtSTASimplified::encapsulate(cPacket *msg)
{
    Ieee80211DataFrame *frame = new Ieee80211DataFrame(msg->getName());

    // frame goes to the AP
    frame->setToDS(true);

    // receiver is the AP
    frame->setReceiverAddress(accessPointAddress);

    // destination address is in address3
    Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(msg->removeControlInfo());
    ASSERT(!ctrl->getDest().isUnspecified());
    frame->setAddress3(ctrl->getDest());
    delete ctrl;

    frame->encapsulate(msg);
    return frame;
}

void Ieee80211MgmtSTASimplified::receiveChangeNotification(int category, const cPolymorphic *details)
{
    Enter_Method_Silent();
    printNotificationBanner(category, details);
}

void Ieee80211MgmtSTASimplified::handleDataFrame(Ieee80211DataFrame *frame)
{
    sendUp(decapsulate(frame));
}

void Ieee80211MgmtSTASimplified::handleAuthenticationFrame(Ieee80211AuthenticationFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtSTASimplified::handleDeauthenticationFrame(Ieee80211DeauthenticationFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtSTASimplified::handleAssociationRequestFrame(Ieee80211AssociationRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtSTASimplified::handleAssociationResponseFrame(Ieee80211AssociationResponseFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtSTASimplified::handleReassociationRequestFrame(Ieee80211ReassociationRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtSTASimplified::handleReassociationResponseFrame(Ieee80211ReassociationResponseFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtSTASimplified::handleDisassociationFrame(Ieee80211DisassociationFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtSTASimplified::handleBeaconFrame(Ieee80211BeaconFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtSTASimplified::handleProbeRequestFrame(Ieee80211ProbeRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtSTASimplified::handleProbeResponseFrame(Ieee80211ProbeResponseFrame *frame)
{
    dropManagementFrame(frame);
}



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


#include "Ieee80211MgmtAdhoc.h"
#include "Ieee802Ctrl_m.h"


Define_Module(Ieee80211MgmtAdhoc);


void Ieee80211MgmtAdhoc::initialize(int stage)
{
    Ieee80211MgmtBase::initialize(stage);
}

void Ieee80211MgmtAdhoc::handleTimer(cMessage *msg)
{
    ASSERT(false);
}

void Ieee80211MgmtAdhoc::handleUpperMessage(cPacket *msg)
{
    Ieee80211DataFrame *frame = encapsulate(msg);
    sendOrEnqueue(frame);
}

void Ieee80211MgmtAdhoc::handleCommand(int msgkind, cPolymorphic *ctrl)
{
    error("handleCommand(): no commands supported");
}

Ieee80211DataFrame *Ieee80211MgmtAdhoc::encapsulate(cPacket *msg)
{
    Ieee80211DataFrame *frame = new Ieee80211DataFrame(msg->getName());

    // copy receiver address from the control info (sender address will be set in MAC)
    Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(msg->removeControlInfo());
    frame->setReceiverAddress(ctrl->getDest());
    delete ctrl;

    frame->encapsulate(msg);
    return frame;
}

void Ieee80211MgmtAdhoc::receiveChangeNotification(int category, const cPolymorphic *details)
{
    Enter_Method_Silent();
    printNotificationBanner(category, details);
}

void Ieee80211MgmtAdhoc::handleDataFrame(Ieee80211DataFrame *frame)
{
    sendUp(decapsulate(frame));
}

void Ieee80211MgmtAdhoc::handleAuthenticationFrame(Ieee80211AuthenticationFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhoc::handleDeauthenticationFrame(Ieee80211DeauthenticationFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhoc::handleAssociationRequestFrame(Ieee80211AssociationRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhoc::handleAssociationResponseFrame(Ieee80211AssociationResponseFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhoc::handleReassociationRequestFrame(Ieee80211ReassociationRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhoc::handleReassociationResponseFrame(Ieee80211ReassociationResponseFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhoc::handleDisassociationFrame(Ieee80211DisassociationFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhoc::handleBeaconFrame(Ieee80211BeaconFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhoc::handleProbeRequestFrame(Ieee80211ProbeRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhoc::handleProbeResponseFrame(Ieee80211ProbeResponseFrame *frame)
{
    dropManagementFrame(frame);
}



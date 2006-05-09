//
// Copyright (C) 2006 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#include "Ieee80211MgmtSimplifiedAP.h"
#include "Ieee802Ctrl_m.h"


Define_Module(Mgmt80211SimplifiedAP);


void Mgmt80211SimplifiedAP::initialize(int)
{
    //...
}

void Mgmt80211SimplifiedAP::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        // TBD process timers
    }
    else
    {
        // process incoming frame
        W80211BasicFrame *frame = check_and_cast<W80211BasicFrame *>(msg);
        processFrame(frame);
        delete frame;
    }
}

void Mgmt80211SimplifiedAP::receiveChangeNotification(int category, cPolymorphic *details)
{
    Enter_Method_Silent();
}

void Mgmt80211SimplifiedAP::handleDataFrame(W80211DataFrame *frame)
{
}

void Mgmt80211SimplifiedAP::handleAuthenticationFrame(W80211AuthenticationFrame *frame)
{
    EV << "ignoring frame " << frame << "\n";
}

void Mgmt80211SimplifiedAP::handleDeauthenticationFrame(W80211DeauthenticationFrame *frame)
{
    EV << "ignoring frame " << frame << "\n";
}

void Mgmt80211SimplifiedAP::handleAssociationRequestFrame(W80211AssociationRequestFrame *frame)
{
    EV << "ignoring frame " << frame << "\n";
}

void Mgmt80211SimplifiedAP::handleAssociationResponseFrame(W80211AssociationResponseFrame *frame)
{
    EV << "ignoring frame " << frame << "\n";
}

void Mgmt80211SimplifiedAP::handleReassociationRequestFrame(W80211ReassociationRequestFrame *frame)
{
    EV << "ignoring frame " << frame << "\n";
}

void Mgmt80211SimplifiedAP::handleReassociationResponseFrame(W80211ReassociationResponseFrame *frame)
{
    EV << "ignoring frame " << frame << "\n";
}

void Mgmt80211SimplifiedAP::handleDisassociationFrame(W80211DisassociationFrame *frame)
{
    EV << "ignoring frame " << frame << "\n";
}

void Mgmt80211SimplifiedAP::handleBeaconFrame(W80211BeaconFrame *frame)
{
    EV << "ignoring frame " << frame << "\n";
}

void Mgmt80211SimplifiedAP::handleProbeRequestFrame(W80211ProbeRequestFrame *frame)
{
    EV << "ignoring frame " << frame << "\n";
}

void Mgmt80211SimplifiedAP::handleProbeResponseFrame(W80211ProbeResponseFrame *frame)
{
    EV << "ignoring frame " << frame << "\n";
}



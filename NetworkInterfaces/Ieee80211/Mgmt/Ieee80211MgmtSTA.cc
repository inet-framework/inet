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


#include "Mgmt80211STA.h"
#include "Ieee802Ctrl_m.h"


Define_Module(Mgmt80211STA);


void Mgmt80211STA::initialize(int)
{
    //...
}

void Mgmt80211STA::handleMessage(cMessage *msg)
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

void Mgmt80211STA::receiveChangeNotification(int category, cPolymorphic *details)
{
}

void Mgmt80211STA::handleDataFrame(W80211DataFrame *frame)
{
}

void Mgmt80211STA::handleAuthenticationFrame(W80211AuthenticationFrame *frame)
{
}

void Mgmt80211STA::handleDeauthenticationFrame(W80211DeauthenticationFrame *frame)
{
}

void Mgmt80211STA::handleAssociationRequestFrame(W80211AssociationRequestFrame *frame)
{
    EV << "ignoring frame " << frame << "\n";
}

void Mgmt80211STA::handleAssociationResponseFrame(W80211AssociationResponseFrame *frame)
{
}

void Mgmt80211STA::handleReassociationRequestFrame(W80211ReassociationRequestFrame *frame)
{
    EV << "ignoring frame " << frame << "\n";
}

void Mgmt80211STA::handleReassociationResponseFrame(W80211ReassociationResponseFrame *frame)
{
}

void Mgmt80211STA::handleDisassociationFrame(W80211DisassociationFrame *frame)
{
}

void Mgmt80211STA::handleBeaconFrame(W80211BeaconFrame *frame)
{
}

void Mgmt80211STA::handleProbeRequestFrame(W80211ProbeRequestFrame *frame)
{
}

void Mgmt80211STA::handleProbeResponseFrame(W80211ProbeResponseFrame *frame)
{
    EV << "ignoring frame " << frame << "\n";
}



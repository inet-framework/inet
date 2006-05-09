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


#include "Ieee80211MgmtSimplifiedSTA.h"
#include "Ieee802Ctrl_m.h"


Define_Module(Ieee80211MgmtSimplifiedSTA);


void Ieee80211MgmtSimplifiedSTA::initialize(int)
{
    //...
}

void Ieee80211MgmtSimplifiedSTA::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        // TBD process timers
    }
    else
    {
        // process incoming frame
        Ieee80211BasicFrame *frame = check_and_cast<Ieee80211BasicFrame *>(msg);
        processFrame(frame);
        delete frame;
    }
}

void Ieee80211MgmtSimplifiedSTA::receiveChangeNotification(int category, cPolymorphic *details)
{
}

void Ieee80211MgmtSimplifiedSTA::handleDataFrame(Ieee80211DataFrame *frame)
{
    //TBD handle
}

void Ieee80211MgmtSimplifiedSTA::handleAuthenticationFrame(Ieee80211AuthenticationFrame *frame)
{
    EV << "ignoring frame " << frame << "\n";
}

void Ieee80211MgmtSimplifiedSTA::handleDeauthenticationFrame(Ieee80211DeauthenticationFrame *frame)
{
    EV << "ignoring frame " << frame << "\n";
}

void Ieee80211MgmtSimplifiedSTA::handleAssociationRequestFrame(Ieee80211AssociationRequestFrame *frame)
{
    EV << "ignoring frame " << frame << "\n";
}

void Ieee80211MgmtSimplifiedSTA::handleAssociationResponseFrame(Ieee80211AssociationResponseFrame *frame)
{
    EV << "ignoring frame " << frame << "\n";
}

void Ieee80211MgmtSimplifiedSTA::handleReassociationRequestFrame(Ieee80211ReassociationRequestFrame *frame)
{
    EV << "ignoring frame " << frame << "\n";
}

void Ieee80211MgmtSimplifiedSTA::handleReassociationResponseFrame(Ieee80211ReassociationResponseFrame *frame)
{
    EV << "ignoring frame " << frame << "\n";
}

void Ieee80211MgmtSimplifiedSTA::handleDisassociationFrame(Ieee80211DisassociationFrame *frame)
{
    EV << "ignoring frame " << frame << "\n";
}

void Ieee80211MgmtSimplifiedSTA::handleBeaconFrame(Ieee80211BeaconFrame *frame)
{
    EV << "ignoring frame " << frame << "\n";
}

void Ieee80211MgmtSimplifiedSTA::handleProbeRequestFrame(Ieee80211ProbeRequestFrame *frame)
{
    EV << "ignoring frame " << frame << "\n";
}

void Ieee80211MgmtSimplifiedSTA::handleProbeResponseFrame(Ieee80211ProbeResponseFrame *frame)
{
    EV << "ignoring frame " << frame << "\n";
}



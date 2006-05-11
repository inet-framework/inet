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


void Ieee80211MgmtSimplifiedSTA::initialize(int stage)
{
    Ieee80211MgmtBase::initialize(stage);
}

void Ieee80211MgmtSimplifiedSTA::handleTimer(cMessage *msg)
{
    ASSERT(false);
}

void Ieee80211MgmtSimplifiedSTA::handleUpperMessage(cMessage *msg)
{
    Ieee80211DataFrame *frame = encapsulate(msg);
    sendOrEnqueue(frame);
}

void Ieee80211MgmtSimplifiedSTA::receiveChangeNotification(int category, cPolymorphic *details)
{
    Enter_Method_Silent();
    EV << "ignoring change notification\n";
}

void Ieee80211MgmtSimplifiedSTA::handleDataFrame(Ieee80211DataFrame *frame)
{
    sendUp(decapsulate(frame));
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



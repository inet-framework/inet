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


#include "Ieee80211MgmtSTA.h"
#include "Ieee802Ctrl_m.h"


Define_Module(Ieee80211MgmtSTA);


void Ieee80211MgmtSTA::initialize(int)
{
    //TBD
}

void Ieee80211MgmtSTA::handleTimer(cMessage *msg)
{
    //TBD
}

void Ieee80211MgmtSTA::handleUpperMessage(cMessage *msg)
{
    //TBD
}

void Ieee80211MgmtSTA::receiveChangeNotification(int category, cPolymorphic *details)
{
    Enter_Method_Silent();
    //TBD
}

void Ieee80211MgmtSTA::handleDataFrame(Ieee80211DataFrame *frame)
{
    //TBD
}

void Ieee80211MgmtSTA::handleAuthenticationFrame(Ieee80211AuthenticationFrame *frame)
{
    //TBD
}

void Ieee80211MgmtSTA::handleDeauthenticationFrame(Ieee80211DeauthenticationFrame *frame)
{
    //TBD
}

void Ieee80211MgmtSTA::handleAssociationRequestFrame(Ieee80211AssociationRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtSTA::handleAssociationResponseFrame(Ieee80211AssociationResponseFrame *frame)
{
    //TBD
}

void Ieee80211MgmtSTA::handleReassociationRequestFrame(Ieee80211ReassociationRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtSTA::handleReassociationResponseFrame(Ieee80211ReassociationResponseFrame *frame)
{
    //TBD
}

void Ieee80211MgmtSTA::handleDisassociationFrame(Ieee80211DisassociationFrame *frame)
{
    //TBD
}

void Ieee80211MgmtSTA::handleBeaconFrame(Ieee80211BeaconFrame *frame)
{
    //TBD
}

void Ieee80211MgmtSTA::handleProbeRequestFrame(Ieee80211ProbeRequestFrame *frame)
{
    //TBD
}

void Ieee80211MgmtSTA::handleProbeResponseFrame(Ieee80211ProbeResponseFrame *frame)
{
    dropManagementFrame(frame);
}



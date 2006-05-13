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


#include "Ieee80211MgmtAP.h"
#include "Ieee802Ctrl_m.h"
#include "EtherFrame_m.h"


Define_Module(Ieee80211MgmtAP);


void Ieee80211MgmtAP::initialize(int stage)
{
    Ieee80211MgmtAPBase::initialize(stage);
}

void Ieee80211MgmtAP::handleTimer(cMessage *msg)
{
    //TBD
}

void Ieee80211MgmtAP::handleUpperMessage(cMessage *msg)
{
    // TBD check we really have a STA with the dest address

    // convert Ethernet frames arriving from MACRelayUnit (i.e. from
    // the AP's other Ethernet or wireless interfaces)
    Ieee80211DataFrame *frame = convertFromEtherFrame(check_and_cast<EtherFrame *>(msg));
    sendOrEnqueue(frame);
}

void Ieee80211MgmtAP::receiveChangeNotification(int category, cPolymorphic *details)
{
    Enter_Method_Silent();
    //TBD
}

void Ieee80211MgmtAP::handleDataFrame(Ieee80211DataFrame *frame)
{
    // check toDS bit
    if (!frame->getToDS())
    {
        // looks like this is not for us - discard
        delete frame;
        return;
    }

    // possibly send frame to the other (Ethernet, etc) ports of the AP as well
    if (hasRelayUnit)
        send(createEtherFrame(frame), "uppergateOut");

    // send it out to the destination STA
    distributeReceivedDataFrame(frame);
}

void Ieee80211MgmtAP::handleAuthenticationFrame(Ieee80211AuthenticationFrame *frame)
{
    //TBD
}

void Ieee80211MgmtAP::handleDeauthenticationFrame(Ieee80211DeauthenticationFrame *frame)
{
    //TBD
}

void Ieee80211MgmtAP::handleAssociationRequestFrame(Ieee80211AssociationRequestFrame *frame)
{
    //TBD
}

void Ieee80211MgmtAP::handleAssociationResponseFrame(Ieee80211AssociationResponseFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAP::handleReassociationRequestFrame(Ieee80211ReassociationRequestFrame *frame)
{
    //TBD
}

void Ieee80211MgmtAP::handleReassociationResponseFrame(Ieee80211ReassociationResponseFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAP::handleDisassociationFrame(Ieee80211DisassociationFrame *frame)
{
    //TBD
}

void Ieee80211MgmtAP::handleBeaconFrame(Ieee80211BeaconFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAP::handleProbeRequestFrame(Ieee80211ProbeRequestFrame *frame)
{
    //TBD
}

void Ieee80211MgmtAP::handleProbeResponseFrame(Ieee80211ProbeResponseFrame *frame)
{
    dropManagementFrame(frame);
}



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


Define_Module(Ieee80211MgmtSimplifiedAP);


void Ieee80211MgmtSimplifiedAP::initialize(int stage)
{
    Ieee80211MgmtBase::initialize(stage);
}

void Ieee80211MgmtSimplifiedAP::handleTimer(cMessage *msg)
{
    ASSERT(false);
}

void Ieee80211MgmtSimplifiedAP::handleUpperMessage(cMessage *msg)
{
    ASSERT(false);  //XXX for now...
    //Ieee80211DataFrame *frame = encapsulate(msg);
    //sendOrEnqueue(frame);
}

void Ieee80211MgmtSimplifiedAP::receiveChangeNotification(int category, cPolymorphic *details)
{
    Enter_Method_Silent();
    EV << "ignoring change notification\n";
}

void Ieee80211MgmtSimplifiedAP::handleDataFrame(Ieee80211DataFrame *frame)
{
    // check toDS bit
    if (!frame->getToDS())
    {
        // looks like this is not for us - discard
        delete frame;
        return;
    }

    // send it out to the destination STA
    distributeDataFrame(frame);
}

void Ieee80211MgmtSimplifiedAP::distributeDataFrame(Ieee80211DataFrame *frame)
{
    // adjust toDS/fromDS bits, and shuffle addresses
    frame->setToDS(false);
    frame->setFromDS(true);

    // move destination address to address1 (receiver address),
    // and fill address3 with original source address;
    // sender address (address2) will be filled in by MAC
    frame->setReceiverAddress(frame->getAddress3());
    frame->setAddress3(frame->getTransmitterAddress());

    sendOut(frame);
}

void Ieee80211MgmtSimplifiedAP::handleAuthenticationFrame(Ieee80211AuthenticationFrame *frame)
{
    dropManagementFrame(frame);;
}

void Ieee80211MgmtSimplifiedAP::handleDeauthenticationFrame(Ieee80211DeauthenticationFrame *frame)
{
    dropManagementFrame(frame);;
}

void Ieee80211MgmtSimplifiedAP::handleAssociationRequestFrame(Ieee80211AssociationRequestFrame *frame)
{
    dropManagementFrame(frame);;
}

void Ieee80211MgmtSimplifiedAP::handleAssociationResponseFrame(Ieee80211AssociationResponseFrame *frame)
{
    dropManagementFrame(frame);;
}

void Ieee80211MgmtSimplifiedAP::handleReassociationRequestFrame(Ieee80211ReassociationRequestFrame *frame)
{
    dropManagementFrame(frame);;
}

void Ieee80211MgmtSimplifiedAP::handleReassociationResponseFrame(Ieee80211ReassociationResponseFrame *frame)
{
    dropManagementFrame(frame);;
}

void Ieee80211MgmtSimplifiedAP::handleDisassociationFrame(Ieee80211DisassociationFrame *frame)
{
    dropManagementFrame(frame);;
}

void Ieee80211MgmtSimplifiedAP::handleBeaconFrame(Ieee80211BeaconFrame *frame)
{
    dropManagementFrame(frame);;
}

void Ieee80211MgmtSimplifiedAP::handleProbeRequestFrame(Ieee80211ProbeRequestFrame *frame)
{
    dropManagementFrame(frame);;
}

void Ieee80211MgmtSimplifiedAP::handleProbeResponseFrame(Ieee80211ProbeResponseFrame *frame)
{
    dropManagementFrame(frame);;
}



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


#include "Ieee80211MgmtSTANoScan.h"
#include "Ieee802Ctrl_m.h"


Define_Module(Ieee80211MgmtSTANoScan);

#define MK_AUTH_TIMEOUT 1


void Ieee80211MgmtSTANoScan::initialize(int stage)
{
    Ieee80211MgmtBase::initialize(stage);
    if (stage==0)
    {
        authenticationTimeout = par("authenticationTimeout");
        associationTimeout = par("associationTimeout");
    }
}

void Ieee80211MgmtSTANoScan::handleTimer(cMessage *msg)
{
    if (msg->kind()==MK_AUTH_TIMEOUT)
    {
        //TBD handle authentication timeout
    }
    else
    {
        error("internal error: unrecognized timer '%s'", msg->name());
    }
}

void Ieee80211MgmtSTANoScan::handleUpperMessage(cMessage *msg)
{
    // XXX revise
    Ieee80211DataFrame *frame = encapsulate(msg);
    sendOrEnqueue(frame);
}

Ieee80211DataFrame *Ieee80211MgmtSTANoScan::encapsulate(cMessage *msg)
{
    Ieee80211DataFrame *frame = new Ieee80211DataFrame(msg->name());

    // frame goes to the AP
    frame->setToDS(true);

    // receiver is the AP
    frame->setReceiverAddress(associateAP.address);

    // destination address is in address3
    Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(msg->removeControlInfo());
    frame->setAddress3(ctrl->getDest());
    delete ctrl;

    frame->encapsulate(msg);
    return frame;
}

void Ieee80211MgmtSTANoScan::startAuthentication(APInfo *ap)
{
    // create and send first authentication frame
    Ieee80211AuthenticationFrame *frame = new Ieee80211AuthenticationFrame("Auth");
    frame->getBody().setSequenceNumber(1);
    // XXX frame length could be increased to account for challenge text length etc.
    sendManagementFrame(frame, ap->address);

    // schedule timeout
    ASSERT(ap->authTimeoutMsg==NULL);
    ap->authTimeoutMsg = new cMessage("authTimeout", MK_AUTH_TIMEOUT);
    ap->authTimeoutMsg->setContextPointer(ap);
    scheduleAt(simTime()+..., ap->authTimeoutMsg);
}

void Ieee80211MgmtSTANoScan::receiveChangeNotification(int category, cPolymorphic *details)
{
    Enter_Method_Silent();
    //TBD
}

void Ieee80211MgmtSTANoScan::handleDataFrame(Ieee80211DataFrame *frame)
{
    sendUp(decapsulate(frame));
}

void Ieee80211MgmtSTANoScan::handleAuthenticationFrame(Ieee80211AuthenticationFrame *frame)
{
    //TBD
}

void Ieee80211MgmtSTANoScan::handleDeauthenticationFrame(Ieee80211DeauthenticationFrame *frame)
{
    //TBD
}

void Ieee80211MgmtSTANoScan::handleAssociationRequestFrame(Ieee80211AssociationRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtSTANoScan::handleAssociationResponseFrame(Ieee80211AssociationResponseFrame *frame)
{
    //TBD
}

void Ieee80211MgmtSTANoScan::handleReassociationRequestFrame(Ieee80211ReassociationRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtSTANoScan::handleReassociationResponseFrame(Ieee80211ReassociationResponseFrame *frame)
{
    //TBD
}

void Ieee80211MgmtSTANoScan::handleDisassociationFrame(Ieee80211DisassociationFrame *frame)
{
    //TBD
}

void Ieee80211MgmtSTANoScan::handleBeaconFrame(Ieee80211BeaconFrame *frame)
{
    //TBD
}

void Ieee80211MgmtSTANoScan::handleProbeRequestFrame(Ieee80211ProbeRequestFrame *frame)
{
    //TBD
}

void Ieee80211MgmtSTANoScan::handleProbeResponseFrame(Ieee80211ProbeResponseFrame *frame)
{
    dropManagementFrame(frame);
}



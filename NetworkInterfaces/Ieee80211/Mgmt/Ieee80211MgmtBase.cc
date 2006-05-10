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


#include "Ieee80211MgmtBase.h"
#include "Ieee802Ctrl_m.h"


static std::ostream& operator<< (std::ostream& out, cMessage *msg)
{
    out << "(" << msg->className() << ")" << msg->fullName();
    return out;
}

void Ieee80211MgmtBase::initialize(int stage)
{
    if (stage==0)
    {
        PassiveQueueBase::initialize();

        queue.setName("80211MacQueue");

        qlenVec.setName("queue length");
        dropVec.setName("drops");

        // configuration
        frameCapacity = par("frameCapacity");
    }
}


void Ieee80211MgmtBase::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        // process timers
        EV << "Timer expired: " << msg << "\n";
        handleTimer(msg);
    }
    else if (msg->arrivedOn("macIn"))
    {
        // process incoming frame
        EV << "Frame arrived from MAC: " << msg << "\n";
        Ieee80211DataOrMgmtFrame *frame = check_and_cast<Ieee80211DataOrMgmtFrame *>(msg);
        processFrame(frame);
        delete frame;
    }
    else
    {
        // packet from upper layers, to be sent out
        EV << "Packet arrived from upper layers: " << msg << "\n";
        handleUpperMessage(msg);
    }
}

void Ieee80211MgmtBase::sendOrEnqueue(cMessage *frame)
{
    PassiveQueueBase::handleMessage(frame);
}

bool Ieee80211MgmtBase::enqueue(cMessage *msg)
{
    if (frameCapacity && queue.length() >= frameCapacity)
    {
        EV << "Queue full, dropping packet.\n";
        delete msg;
        dropVec.record(1);
        return true;
    }
    else
    {
        queue.insert(msg);
        qlenVec.record(queue.length());
        return false;
    }
}

cMessage *Ieee80211MgmtBase::dequeue()
{
    if (queue.empty())
        return NULL;

   cMessage *pk = (cMessage *)queue.pop();

    // statistics
    qlenVec.record(queue.length());
    return pk;
}

void Ieee80211MgmtBase::sendOut(cMessage *msg)
{
    send(msg, "macOut");
}

Ieee80211DataFrame *Ieee80211MgmtBase::encapsulate(cMessage *msg)
{
    if (msg->byteLength() > 2312)
        error("message from higher layer (%s)%s is too long for 802.11b, %d bytes (fragmentation is not supported yet)",
              msg->className(), msg->name(), msg->byteLength());

    Ieee80211DataFrame *frame = new Ieee80211DataFrame(msg->name());

    // copy dest address from the control info
    Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(msg->removeControlInfo());
    frame->setReceiverAddress(ctrl->getDest());
    delete ctrl;

    // set the src address to our mac address -- MAC will do this
    //frame->setTransmitterAddress(address);

    frame->encapsulate(msg);
    return frame;
}

cMessage *Ieee80211MgmtBase::decapsulate(Ieee80211DataFrame *frame)
{
    //XXX create control info with src address?
    return frame->decapsulate();
}

void Ieee80211MgmtBase::processFrame(Ieee80211DataOrMgmtFrame *frame)
{
    const Ieee80211FrameControl& frameControl = frame->getFrameControl();
    switch(frameControl.subtype)
    {
      case ST_DATA:
        handleDataFrame(check_and_cast<Ieee80211DataFrame *>(frame));
        break;
      case ST_AUTHENTICATION:
        handleAuthenticationFrame(check_and_cast<Ieee80211AuthenticationFrame *>(frame));
        break;
      case ST_DEAUTHENTICATION:
        handleDeauthenticationFrame(check_and_cast<Ieee80211DeauthenticationFrame *>(frame));
        break;
      case ST_ASSOCIATIONREQUEST:
        handleAssociationRequestFrame(check_and_cast<Ieee80211AssociationRequestFrame *>(frame));
        break;
      case ST_ASSOCIATIONRESPONSE:
        handleAssociationResponseFrame(check_and_cast<Ieee80211AssociationResponseFrame *>(frame));
        break;
      case ST_REASSOCIATIONREQUEST:
        handleReassociationRequestFrame(check_and_cast<Ieee80211ReassociationRequestFrame *>(frame));
        break;
      case ST_REASSOCIATIONRESPONSE:
        handleReassociationResponseFrame(check_and_cast<Ieee80211ReassociationResponseFrame *>(frame)); break;
      case ST_DISASSOCIATION:
        handleDisassociationFrame(check_and_cast<Ieee80211DisassociationFrame *>(frame));
        break;
      case ST_BEACON:
        handleBeaconFrame(check_and_cast<Ieee80211BeaconFrame *>(frame));
        break;
      case ST_PROBEREQUEST:
        handleProbeRequestFrame(check_and_cast<Ieee80211ProbeRequestFrame *>(frame));
        break;
      case ST_PROBERESPONSE:
        handleProbeResponseFrame(check_and_cast<Ieee80211ProbeResponseFrame *>(frame));
        break;
      default:
        error("unexpected frame type (%s)%s", frame->className(), frame->name());
    }
}


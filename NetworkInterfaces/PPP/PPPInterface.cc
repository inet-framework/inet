//
// Copyright (C) 2004 Andras Varga
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

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "PPPFrame_m.h"


/**
 * PPP implementation. Derived from the p-to-p OMNeT++ sample simulation.
 */
class PPPInterface : public cSimpleModule
{
  protected:
    long frameCapacity;
    long bitCapacity;
    cGate *gateToWatch;

    cQueue queue;
    cMessage *endTransmissionEvent;

  public:
    Module_Class_Members(PPPInterface, cSimpleModule, 0);

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    void startTransmitting(cMessage *msg);
    PPPFrame *encapsulate(cMessage *msg);
    cMessage *decapsulate(PPPFrame *pppFrame);
};

Define_Module(PPPInterface);

void PPPInterface::initialize()
{
    queue.setName("queue");
    endTransmissionEvent = new cMessage("pppEndTxEvent");

    frameCapacity = par("frameCapacity");
    bitCapacity = par("bitCapacity");

    // get the 1st one with transmission rate
    gateToWatch = gate("physOut");
    while (gateToWatch)
    {
        // does this gate have data rate?
        cSimpleChannel *chan = dynamic_cast<cSimpleChannel*>(gateToWatch->channel());
        if (chan && chan->datarate()>0)
            break;
        // otherwise just check next connection in path
        gateToWatch = gateToWatch->toGate();
    }
    if (!gateToWatch)
        error("gate physicalOut must be connected (directly or indirectly) to a link with data rate");
}


void PPPInterface::startTransmitting(cMessage *msg)
{
    PPPFrame *pppFrame = encapsulate(msg);
    if (ev.isGUI()) displayString().setTagArg("i",1, queue.length()>=3 ? "red" : "yellow");

    ev << "Starting transmission of " << pppFrame << endl;
    send(pppFrame, "physOut");

    // schedule an event for the time when last bit will leave the gate.
    simtime_t endTransmission = gateToWatch->transmissionFinishes();
    scheduleAt(endTransmission, endTransmissionEvent);
}

void PPPInterface::handleMessage(cMessage *msg)
{
    if (msg==endTransmissionEvent)
    {
        // Transmission finished, we can start next one.
        ev << "Transmission finished.\n";
        if (ev.isGUI()) displayString().setTagArg("i",1,"");
        if (!queue.empty())
        {
            msg = (cMessage *) queue.getTail();
            startTransmitting(msg);
        }
    }
    else if (msg->arrivedOn("physIn"))
    {
        // check for bit errors
        if (msg->hasBitError())
        {
            ev << "Bit error in " << msg << endl;
            delete msg;
            return;
        }

        // pass up payload
        cMessage *payload = decapsulate(check_and_cast<PPPFrame *>(msg));
        send(payload,"netwOut");
    }
    else // arrived on gate "in"
    {
        if (endTransmissionEvent->isScheduled())
        {
            // We are currently busy, so just queue up the packet.
            ev << "Received " << msg << " but transmitter busy, queueing.\n";
            queue.insert(msg);  // FIXME use frameCapacity, bitCapacity
        }
        else
        {
            // We are idle, so we can start transmitting right away.
            ev << "Received " << msg << endl;
            startTransmitting(msg);
        }
    }
}

PPPFrame *PPPInterface::encapsulate(cMessage *msg)
{
    PPPFrame *pppFrame = new PPPFrame(msg->name());
    pppFrame->setLength(8*PPP_OVERHEAD_BYTES);
    pppFrame->encapsulate(msg);
    return pppFrame;
}

cMessage *PPPInterface::decapsulate(PPPFrame *pppFrame)
{
    cMessage *msg = pppFrame->decapsulate();
    delete pppFrame;
    return msg;
}



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
#include "RoutingTable.h"
#include "RoutingTableAccess.h"


/**
 * PPP implementation. Derived from the p-to-p OMNeT++ sample simulation.
 */
class PPPInterface : public cSimpleModule
{
  protected:
    bool connected;

    long frameCapacity;
    long bitCapacity;
    cGate *gateToWatch;

    cQueue queue;
    cMessage *endTransmissionEvent;

    void registerInterface();
    void startTransmitting(cMessage *msg);
    PPPFrame *encapsulate(cMessage *msg);
    cMessage *decapsulate(PPPFrame *pppFrame);

  public:
    Module_Class_Members(PPPInterface, cSimpleModule, 0);

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(PPPInterface);

void PPPInterface::initialize()
{
    queue.setName("queue");
    endTransmissionEvent = new cMessage("pppEndTxEvent");

    frameCapacity = par("frameCapacity");
    bitCapacity = par("bitCapacity");

    // if we're connected, get the gatee with transmission rate
    gateToWatch = gate("physOut");
    connected = false;
    if (gateToWatch->destinationGate()->type()=='I')
    {
        connected = true;
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

    // if not connected, make it gray
    if (!connected && ev.isGUI())
    {
        displayString().setTagArg("i",1,"#707070");
        displayString().setTagArg("i",2,"100");
        displayString().setTagArg("t",0,"unconnected");
    }

    // register our interface entry in RoutingTable
    registerInterface();
}

void PPPInterface::registerInterface()
{
    InterfaceEntry *e = new InterfaceEntry();

    // interface name: our module name without special characters ([])
    char *interfaceName = new char[strlen(fullName())+1];
    char *d=interfaceName;
    for (const char *s=fullName(); *s; s++)
        if (isalnum(*s))
            *d++ = *s;
    *d = '\0';

    e->name = interfaceName;
    delete [] interfaceName;

    // output port: index of gate where our "physOut" is connected
    int outputPort = gate("physOut")->toGate()->index();  // FIXME use queueIn instead!!!
    e->outputPort = outputPort;

    // we don't know IP address and netmask, it'll probably come from routing table file

    // MTU: typical values are 576 (Internet de facto), 1500 (Ethernet-friendly),
    // 4000 (on some point-to-point links), 4470 (Cisco routers default, FDDI compatible)
    e->mtu = 4470;

    // metric: some hints: OSPF cost (2e9/bps value), MS KB article Q299540, ...
    e->metric = 100;  // default
    if (connected)
    {
        cSimpleChannel *chan = dynamic_cast<cSimpleChannel*>(gateToWatch->channel());
        double bps = chan->datarate();
        e->metric = (int)ceil(2e9/bps); // use OSPF cost as default
    }

    // capabilities
    e->multicast = true;
    e->pointToPoint = true;

    // multicast groups
    //FIXME

    // add
    RoutingTableAccess routingTableAccess;
    routingTableAccess.get()->addInterface(e);
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
    if (!connected)
    {
        ev << "Interface is not connected, dropping packet " << msg << endl;
        delete msg;
        return;
    }

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



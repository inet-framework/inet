/*
 * Copyright (C) 2003 CTIE, Monash University
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include <omnetpp.h>
#include "EtherFrame_m.h"  // for EtherAutoconfig only
#include "utils.h"



/**
 * Models a wiring hub. It simply broadcasts the received message
 * on all other ports.
 */
class EtherHub : public cSimpleModule
{
    Module_Class_Members(EtherHub,cSimpleModule,0);

    virtual void initialize();
    virtual void handleMessage(cMessage*);
    virtual void finish();

  private:
    int ports;          // number of ports
    long numMessages;   // number of messages handled

};

Define_Module(EtherHub);

static cEnvir& operator<< (cEnvir& ev, cMessage *msg)
{
    ev.printf("(%s)%s",msg->className(),msg->fullName());
    return ev;
}

void EtherHub::initialize()
{
    numMessages = 0;
    WATCH(numMessages);

    ports = gate("in",0)->size();
    if (gate("out",0)->size()!=ports)
        error("the sizes of the in[] and out[] gate vectors must be the same");


    // autoconfig: tell everyone that full duplex is not possible over shared media
    EV << "Autoconfig: advertising that we only support half-duplex operation\n";
    for (int i=0; i<ports; i++)
    {
        EtherAutoconfig *autoconf = new EtherAutoconfig("autoconf-halfduplex");
        autoconf->setHalfDuplex(true);
        send(autoconf,"out",i);
    }
}

void EtherHub::handleMessage(cMessage *msg)
{
    // Handle frame sent down from the network entity: send out on every other port
    int arrivalPort = msg->arrivalGate()->index();
    EV << "Frame " << msg << " arrived on port " << arrivalPort << ", broadcasting on all other ports\n";

    numMessages++;

    if (ports<=1)
    {
        delete msg;
        return;
    }
    for (int i=0; i<ports; i++)
    {
        if (i!=arrivalPort)
        {
            bool isLast = (arrivalPort==ports-1) ? (i==ports-2) : (i==ports-1);
            cMessage *msg2 = isLast ? msg : (cMessage*) msg->dup();
            send(msg2,"out",i);
        }
    }
}

void EtherHub::finish ()
{
    if (par("writeScalars").boolValue())
    {
        double t = simTime();
        recordScalar("simulated time", t);
        recordScalar("messages handled", numMessages);
        if (t>0)
            recordScalar("messages/sec", numMessages/t);
    }
}


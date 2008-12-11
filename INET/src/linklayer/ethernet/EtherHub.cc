/*
 * Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#include "EtherHub.h"
#include "EtherFrame_m.h"  // for EtherAutoconfig only


Define_Module(EtherHub);


static cEnvir& operator<< (cEnvir& out, cMessage *msg)
{
    out.printf("(%s)%s",msg->getClassName(),msg->getFullName());
    return out;
}

void EtherHub::initialize()
{
    numMessages = 0;
    WATCH(numMessages);

    ports = gateSize("ethg");

    // autoconfig: tell everyone that full duplex is not possible over shared media
    EV << "Autoconfig: advertising that we only support half-duplex operation\n";
    for (int i=0; i<ports; i++)
    {
        EtherAutoconfig *autoconf = new EtherAutoconfig("autoconf-halfduplex");
        autoconf->setHalfDuplex(true);
        send(autoconf,"ethg$o",i);
    }
}

void EtherHub::handleMessage(cMessage *msg)
{
    // Handle frame sent down from the network entity: send out on every other port
    int arrivalPort = msg->getArrivalGate()->getIndex();
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
            send(msg2,"ethg$o",i);
        }
    }
}

void EtherHub::finish ()
{
    simtime_t t = simTime();
    recordScalar("simulated time", t);
    recordScalar("messages handled", numMessages);
    if (t>0)
        recordScalar("messages/sec", numMessages/t);
}


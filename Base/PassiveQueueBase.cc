//
// Copyright (C) 2005 Andras Varga
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


#include <omnetpp.h>
#include "PassiveQueueBase.h"


void PassiveQueueBase::initialize()
{
    // state
    packetRequested = 0;

    // statistics
    numReceived = 0;
    numDropped = 0;
    WATCH(numReceived);
    WATCH(numDropped);
}

void PassiveQueueBase::handleMessage(cMessage *msg)
{
    numReceived++;
    if (packetRequested>0)
    {
        packetRequested--;
        sendOut(msg);
    }
    else
    {
        bool dropped = enqueue(msg);
        if (dropped)
            numDropped++;
    }

    if (ev.isGUI())
    {
        char buf[40];
        sprintf(buf, "rcvd: %d pks\ndropped: %d pks", numReceived, numDropped);
        displayString().setTagArg("t",0,buf);
    }
}

void PassiveQueueBase::requestPacket()
{
    Enter_Method("requestPacket()");

    cMessage *msg = dequeue();
    if (msg==NULL)
    {
        packetRequested++;
    }
    else
    {
        sendOut(msg);
    }
}

void PassiveQueueBase::finish()
{
    recordScalar("packets received", numReceived);
    recordScalar("packets dropped", numDropped);
}


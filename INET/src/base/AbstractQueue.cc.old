//
// Copyright (C) 2004, 2008 Andras Varga
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
#include "AbstractQueue.h"


AbstractQueue::AbstractQueue()
{
    msgBeingServiced = endServiceTimer = NULL;
}

AbstractQueue::~AbstractQueue()
{
    delete msgBeingServiced;
    cancelAndDelete(endServiceTimer);
}

void AbstractQueue::initialize()
{
    msgBeingServiced = NULL;
    endServiceTimer = new cMessage("end-service");
    queue.setName("queue");
}

void AbstractQueue::handleMessage(cMessage *msg)
{
    if (msg==endServiceTimer)
    {
        cMessage *tmp = msgBeingServiced;
        msgBeingServiced = NULL;
        doEndService(tmp);
    }
    else if (!msgBeingServiced)
    {
        cMessage *msg2 = arrivalWhenIdle(msg);
        if (msg2)
            doStartService(msg2);
    }
    else
    {
        arrival(msg);
    }
}

void AbstractQueue::doStartService(cMessage *msg)
{
    simtime_t serviceTime = startService(msg);
    if (serviceTime != 0) {
        msgBeingServiced = msg;
        scheduleAt(simTime()+serviceTime, endServiceTimer);
    }
    else {
        doEndService(msg);
    }
}

void AbstractQueue::doEndService(cMessage *msg)
{
    endService(msg);
    if (!queue.empty())
        doStartService(queue.pop());
}


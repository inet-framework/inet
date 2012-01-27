//
// Copyright (C) 2004, 2008 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#include "INETDefs.h"

#include "AbstractQueue.h"


AbstractQueue::AbstractQueue()
{
    msgServiced = NULL;
    endServiceMsg = NULL;
}

AbstractQueue::~AbstractQueue()
{
    delete msgServiced;
    cancelAndDelete(endServiceMsg);
}

void AbstractQueue::initialize()
{
    msgServiced = NULL;
    endServiceMsg = new cMessage("end-service");
    queue.setName("queue");
}

void AbstractQueue::handleMessage(cMessage *msg)
{
    if (msg==endServiceMsg)
    {
        doEndService();
    }
    else if (!msgServiced)
    {
        cPacket *msg2 = arrivalWhenIdle( PK(msg) );
        if (msg2)
        {
            msgServiced = msg2;
            doStartService();
        }

    }
    else
    {
        arrival( PK(msg) );
    }
}

void AbstractQueue::doStartService()
{
    simtime_t serviceTime = startService( msgServiced );
    if (serviceTime != 0)
        scheduleAt( simTime()+serviceTime, endServiceMsg );
    else
        doEndService();
}

void AbstractQueue::doEndService()
{
    endService( msgServiced );
    if (queue.empty())
    {
        msgServiced = NULL;
    }
    else
    {
        msgServiced = queue.pop();
        doStartService();
    }
}


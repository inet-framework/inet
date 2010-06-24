//
// Copyright (C) 2005 Andras Varga
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


#include <omnetpp.h>
#include "DropTailQueue.h"


Define_Module(DropTailQueue);

void DropTailQueue::initialize()
{
    PassiveQueueBase::initialize();
    queue.setName("l2queue");

    //statistics
    queueLengthSignal = registerSignal("queueLength");
    emit(queueLengthSignal, 0L);

    outGate = gate("out");

    // configuration
    frameCapacity = par("frameCapacity");
}

bool DropTailQueue::enqueue(cMessage *msg)
{
    if (frameCapacity && queue.length() >= frameCapacity)
    {
        EV << "Queue full, dropping packet.\n";
        delete msg;
        return true;
    }
    else
    {
        queue.insert(msg);
        emit(queueLengthSignal, (long)(queue.length()));
        return false;
    }
}

cMessage *DropTailQueue::dequeue()
{
    if (queue.empty())
        return NULL;

   cMessage *pk = (cMessage *)queue.pop();

    // statistics
    emit(queueLengthSignal, (long)(queue.length()));

    return pk;
}

void DropTailQueue::sendOut(cMessage *msg)
{
    send(msg, outGate);
}



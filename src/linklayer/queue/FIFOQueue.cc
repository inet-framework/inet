//
// Copyright (C) 2012 Opensim Ltd.
// Author: Tamas Borbely
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

#include "FIFOQueue.h"


Define_Module(FIFOQueue);

simsignal_t FIFOQueue::queueLengthSignal = SIMSIGNAL_NULL;

void FIFOQueue::initialize()
{
    PassiveQueueBase::initialize();
    queue.setName(par("queueName"));
    outGate = gate("out");
    queueLengthSignal = registerSignal("queueLength");
}

cMessage *FIFOQueue::enqueue(cMessage *msg)
{
    cPacket *packet = check_and_cast<cPacket*>(msg);
    queue.insert(packet);
    byteLength += packet->getByteLength();
    emit(queueLengthSignal, queue.length());
    return NULL;
}

cMessage *FIFOQueue::dequeue()
{
    if (queue.empty())
        return NULL;

    cPacket *packet = check_and_cast<cPacket*>(queue.pop());
    byteLength -= packet->getByteLength();
    emit(queueLengthSignal, queue.length());
    return packet;
}

void FIFOQueue::sendOut(cMessage *msg)
{
    send(msg, outGate);
}

bool FIFOQueue::isEmpty()
{
    return queue.empty();
}


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

#include "inet/common/INETDefs.h"

#include "inet/common/queue/DropTailQueue.h"

namespace inet {

Define_Module(DropTailQueue);

simsignal_t DropTailQueue::queueLengthSignal = registerSignal("queueLength");

void DropTailQueue::initialize()
{
    PassiveQueueBase::initialize();

    queue.setName(par("queueName"));

    //statistics
    emit(queueLengthSignal, queue.getLength());

    outGate = gate("out");

    // configuration
    frameCapacity = par("frameCapacity");
}

cMessage *DropTailQueue::enqueue(cMessage *msg)
{
    if (frameCapacity && queue.getLength() >= frameCapacity) {
        EV << "Queue full, dropping packet.\n";
        return msg;
    }
    else {
        queue.insert(msg);
        emit(queueLengthSignal, queue.getLength());
        return nullptr;
    }
}

cMessage *DropTailQueue::dequeue()
{
    if (queue.isEmpty())
        return nullptr;

    cMessage *msg = (cMessage *)queue.pop();

    // statistics
    emit(queueLengthSignal, queue.getLength());

    return msg;
}

void DropTailQueue::sendOut(cMessage *msg)
{
    send(msg, outGate);
}

bool DropTailQueue::isEmpty()
{
    return queue.isEmpty();
}

} // namespace inet


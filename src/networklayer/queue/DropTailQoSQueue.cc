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


#include "INETDefs.h"

#include "DropTailQoSQueue.h"


Define_Module(DropTailQoSQueue);

DropTailQoSQueue::DropTailQoSQueue()
{
    queues = NULL;
    numQueues = 0;
}

DropTailQoSQueue::~DropTailQoSQueue()
{
    for (int i=0; i<numQueues; i++)
        delete queues[i];
    delete [] queues;
}

void DropTailQoSQueue::initialize()
{
    PassiveQueueBase::initialize();

    // configuration
    frameCapacity = par("frameCapacity");

    const char *classifierClass = par("classifierClass");
    classifier = check_and_cast<IQoSClassifier *>(createOne(classifierClass));

    outGate = gate("out");

    numQueues = classifier->getNumQueues();
    queues = new cQueue *[numQueues];
    for (int i=0; i<numQueues; i++)
    {
        char buf[32];
        sprintf(buf, "queue-%d", i);
        queues[i] = new cQueue(buf);
    }
}

cMessage *DropTailQoSQueue::enqueue(cMessage *msg)
{
    int queueIndex = classifier->classifyPacket(msg);
    cQueue *queue = queues[queueIndex];

    if (frameCapacity && queue->length() >= frameCapacity)
    {
        EV << "Queue " << queueIndex << " full, dropping packet.\n";
        return msg;
    }
    else
    {
        queue->insert(msg);
        return NULL;
    }
}

cMessage *DropTailQoSQueue::dequeue()
{
    // queue 0 is highest priority
    for (int i=0; i<numQueues; i++)
    {
        if (!queues[i]->empty())
            return (cMessage *)queues[i]->pop();
    }
    return NULL;
}

void DropTailQoSQueue::sendOut(cMessage *msg)
{
    send(msg, outGate);
}

bool DropTailQoSQueue::isEmpty()
{
    // queue 0 is highest priority
    for (int i=0; i<numQueues; i++)
        if (!queues[i]->empty())
            return false;
    return true;
}

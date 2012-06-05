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

#include <algorithm>

#include "PassiveQueueBase.h"

simsignal_t PassiveQueueBase::rcvdPkSignal = SIMSIGNAL_NULL;
simsignal_t PassiveQueueBase::enqueuePkSignal = SIMSIGNAL_NULL;
simsignal_t PassiveQueueBase::dequeuePkSignal = SIMSIGNAL_NULL;
simsignal_t PassiveQueueBase::dropPkByQueueSignal = SIMSIGNAL_NULL;
simsignal_t PassiveQueueBase::queueingTimeSignal = SIMSIGNAL_NULL;

void PassiveQueueBase::initialize()
{
    // state
    packetRequested = 0;
    WATCH(packetRequested);

    // statistics
    numQueueReceived = 0;
    numQueueDropped = 0;
    WATCH(numQueueReceived);
    WATCH(numQueueDropped);

    rcvdPkSignal = registerSignal("rcvdPk");
    enqueuePkSignal = registerSignal("enqueuePk");
    dequeuePkSignal = registerSignal("dequeuePk");
    dropPkByQueueSignal = registerSignal("dropPkByQueue");
    queueingTimeSignal = registerSignal("queueingTime");

    msgId2TimeMap.clear();
}

void PassiveQueueBase::handleMessage(cMessage *msg)
{
    numQueueReceived++;

    emit(rcvdPkSignal, msg);

    if (packetRequested > 0)
    {
        packetRequested--;
        emit(enqueuePkSignal, msg);
        emit(dequeuePkSignal, msg);
        emit(queueingTimeSignal, 0L);
        sendOut(msg);
    }
    else
    {
        msgId2TimeMap[msg->getId()] = simTime();
        cMessage *droppedMsg = enqueue(msg);
        if (msg != droppedMsg)
            emit(enqueuePkSignal, msg);

        if (droppedMsg)
        {
            numQueueDropped++;
            emit(dropPkByQueueSignal, droppedMsg);
            msgId2TimeMap.erase(droppedMsg->getId());
            delete droppedMsg;
        }
        else
            notifyListeners();
    }

    if (ev.isGUI())
    {
        char buf[40];
        sprintf(buf, "q rcvd: %d\nq dropped: %d", numQueueReceived, numQueueDropped);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

void PassiveQueueBase::requestPacket()
{
    Enter_Method("requestPacket()");

    cMessage *msg = dequeue();
    if (msg == NULL)
    {
        packetRequested++;
    }
    else
    {
        emit(dequeuePkSignal, msg);
        emit(queueingTimeSignal, simTime() - msgId2TimeMap[msg->getId()]);
        msgId2TimeMap.erase(msg->getId());
        sendOut(msg);
    }
}

void PassiveQueueBase::clear()
{
    cMessage *msg;

    while (NULL != (msg = dequeue()))
        delete msg;

    packetRequested = 0;
}

void PassiveQueueBase::finish()
{
    msgId2TimeMap.clear();
}

void PassiveQueueBase::addListener(IPassiveQueueListener *listener)
{
    std::list<IPassiveQueueListener*>::iterator it = find(listeners.begin(), listeners.end(), listener);
    if (it == listeners.end())
        listeners.push_back(listener);
}

void PassiveQueueBase::removeListener(IPassiveQueueListener *listener)
{
    std::list<IPassiveQueueListener*>::iterator it = find(listeners.begin(), listeners.end(), listener);
    if (it != listeners.end())
        listeners.erase(it);
}

void PassiveQueueBase::notifyListeners()
{
    for (std::list<IPassiveQueueListener*>::iterator it = listeners.begin(); it != listeners.end(); ++it)
        (*it)->packetEnqueued(this);
}


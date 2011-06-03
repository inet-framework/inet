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
#include "PassiveQueueBase.h"

simsignal_t PassiveQueueBase::rcvdPkBytesSignal = SIMSIGNAL_NULL;
simsignal_t PassiveQueueBase::sentPkBytesSignal = SIMSIGNAL_NULL;
simsignal_t PassiveQueueBase::droppedPkBytesSignal = SIMSIGNAL_NULL;
simsignal_t PassiveQueueBase::queueingTimeSignal = SIMSIGNAL_NULL;

static long getMsgByteLength(cMessage* msg)
{
    if (msg->isPacket())
        return (long)(((cPacket*)msg)->getByteLength());

    return 0; // Unknown value
}

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

    rcvdPkBytesSignal = registerSignal("rcvdPkBytes");
    sentPkBytesSignal = registerSignal("sentPkBytes");
    droppedPkBytesSignal = registerSignal("droppedPkBytes");
    queueingTimeSignal = registerSignal("queueingTime");

    msgId2TimeMap.clear();
}

void PassiveQueueBase::handleMessage(cMessage *msg)
{
    numQueueReceived++;

    long msgBytes = getMsgByteLength(msg);

    emit(rcvdPkBytesSignal, msgBytes);

    if (packetRequested > 0)
    {
        packetRequested--;
        emit(sentPkBytesSignal, msgBytes);
        emit(queueingTimeSignal, 0L);
        sendOut(msg);
    }
    else
    {
        msgId2TimeMap[msg->getId()] = simTime();
        cMessage *droppedMsg = enqueue(msg);
        if (droppedMsg)
        {
            numQueueDropped++;
            emit(droppedPkBytesSignal, getMsgByteLength(droppedMsg));
            msgId2TimeMap.erase(droppedMsg->getId());
            delete droppedMsg;
        }
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
        emit(sentPkBytesSignal, getMsgByteLength(msg));
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


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

#include "inet/common/queue/PassiveQueueBase.h"

namespace inet {

simsignal_t PassiveQueueBase::rcvdPkSignal = registerSignal("rcvdPk");
simsignal_t PassiveQueueBase::enqueuePkSignal = registerSignal("enqueuePk");
simsignal_t PassiveQueueBase::dequeuePkSignal = registerSignal("dequeuePk");
simsignal_t PassiveQueueBase::dropPkByQueueSignal = registerSignal("dropPkByQueue");
simsignal_t PassiveQueueBase::queueingTimeSignal = registerSignal("queueingTime");

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
}

void PassiveQueueBase::handleMessage(cMessage *msg)
{
    numQueueReceived++;

    emit(rcvdPkSignal, msg);

    if (packetRequested > 0) {
        packetRequested--;
        emit(enqueuePkSignal, msg);
        emit(dequeuePkSignal, msg);
        emit(queueingTimeSignal, SIMTIME_ZERO);
        sendOut(msg);
    }
    else {
        msg->setArrivalTime(simTime());
        cMessage *droppedMsg = enqueue(msg);
        if (msg != droppedMsg)
            emit(enqueuePkSignal, msg);

        if (droppedMsg) {
            numQueueDropped++;
            emit(dropPkByQueueSignal, droppedMsg);
            delete droppedMsg;
        }
        else
            notifyListeners();
    }

    if (hasGUI()) {
        char buf[40];
        sprintf(buf, "q rcvd: %d\nq dropped: %d", numQueueReceived, numQueueDropped);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

void PassiveQueueBase::requestPacket()
{
    Enter_Method("requestPacket()");

    cMessage *msg = dequeue();
    if (msg == nullptr) {
        packetRequested++;
    }
    else {
        emit(dequeuePkSignal, msg);
        emit(queueingTimeSignal, simTime() - msg->getArrivalTime());
        sendOut(msg);
    }
}

void PassiveQueueBase::clear()
{
    cMessage *msg;

    while (nullptr != (msg = dequeue()))
        delete msg;

    packetRequested = 0;
}

cMessage *PassiveQueueBase::pop()
{
    return dequeue();
}

void PassiveQueueBase::finish()
{
}

void PassiveQueueBase::addListener(IPassiveQueueListener *listener)
{
    auto it = find(listeners.begin(), listeners.end(), listener);
    if (it == listeners.end())
        listeners.push_back(listener);
}

void PassiveQueueBase::removeListener(IPassiveQueueListener *listener)
{
    auto it = find(listeners.begin(), listeners.end(), listener);
    if (it != listeners.end())
        listeners.erase(it);
}

void PassiveQueueBase::notifyListeners()
{
    for (auto & elem : listeners)
        (elem)->packetEnqueued(this);
}

} // namespace inet


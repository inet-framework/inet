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

#include <algorithm>

#include "inet/common/queue/SchedulerBase.h"

namespace inet {

SchedulerBase::SchedulerBase()
    : packetsRequestedFromUs(0), packetsToBeRequestedFromInputs(0), outGate(nullptr)
{
}

SchedulerBase::~SchedulerBase()
{
}

void SchedulerBase::initialize()
{
    int numInputs = gateSize("in");
    for (int i = 0; i < numInputs; ++i) {
        cGate *inGate = isGateVector("in") ? gate("in", i) : gate("in");
        cGate *connectedGate = inGate->getPathStartGate();
        if (!connectedGate)
            throw cRuntimeError("Scheduler input gate %d is not connected", i);
        IPassiveQueue *inputModule = dynamic_cast<IPassiveQueue *>(connectedGate->getOwnerModule());
        if (!inputModule)
            throw cRuntimeError("Scheduler input gate %d should be connected to an IPassiveQueue", i);
        inputModule->addListener(this);
        inputQueues.push_back(inputModule);
    }

    outGate = gate("out");

    // TODO update state when topology changes
}

void SchedulerBase::finalize()
{
    for (auto & elem : inputQueues)
        (elem)->removeListener(this);
}

void SchedulerBase::handleMessage(cMessage *msg)
{
    ASSERT(packetsRequestedFromUs > 0);
    packetsRequestedFromUs--;
    sendOut(msg);
}

void SchedulerBase::packetEnqueued(IPassiveQueue *inputQueue)
{
    Enter_Method("packetEnqueued(...)");

    if (packetsToBeRequestedFromInputs > 0) {
        bool success = schedulePacket();
        if (success)
            packetsToBeRequestedFromInputs--;
    }
    else if (packetsRequestedFromUs == 0)
        notifyListeners();
}

void SchedulerBase::requestPacket()
{
    Enter_Method("requestPacket()");

    packetsRequestedFromUs++;
    packetsToBeRequestedFromInputs++;
    bool success = schedulePacket();
    if (success)
        packetsToBeRequestedFromInputs--;
}

void SchedulerBase::sendOut(cMessage *msg)
{
    send(msg, outGate);
}

bool SchedulerBase::isEmpty()
{
    for (auto & elem : inputQueues)
        if (!(elem)->isEmpty())
            return false;

    return true;
}

void SchedulerBase::clear()
{
    for (auto & elem : inputQueues)
        (elem)->clear();

    packetsRequestedFromUs = 0;
    packetsToBeRequestedFromInputs = 0;
}

cMessage *SchedulerBase::pop()
{
    for (auto & elem : inputQueues)
        if (!(elem)->isEmpty())
            return (elem)->pop();

    return nullptr;
}

void SchedulerBase::addListener(IPassiveQueueListener *listener)
{
    auto it = find(listeners.begin(), listeners.end(), listener);
    if (it == listeners.end())
        listeners.push_back(listener);
}

void SchedulerBase::removeListener(IPassiveQueueListener *listener)
{
    auto it = find(listeners.begin(), listeners.end(), listener);
    if (it != listeners.end())
        listeners.erase(it);
}

void SchedulerBase::notifyListeners()
{
    for (auto & elem : listeners)
        (elem)->packetEnqueued(this);
}

} // namespace inet


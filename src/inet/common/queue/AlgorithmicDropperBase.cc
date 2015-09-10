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

#include "inet/common/queue/AlgorithmicDropperBase.h"

namespace inet {

void AlgorithmicDropperBase::initialize()
{
    numGates = gateSize("out");
    for (int i = 0; i < numGates; ++i) {
        cGate *outGate = gate("out", i);
        cGate *connectedGate = outGate->getPathEndGate();
        if (!connectedGate)
            throw cRuntimeError("ThresholdDropper out gate %d is not connected", i);
        IQueueAccess *outModule = dynamic_cast<IQueueAccess *>(connectedGate->getOwnerModule());
        if (!outModule)
            throw cRuntimeError("ThresholdDropper out gate %d should be connected a simple module implementing IQueueAccess", i);
        outQueues.push_back(outModule);
        outQueueSet.insert(outModule);
    }
}

void AlgorithmicDropperBase::handleMessage(cMessage *msg)
{
    cPacket *packet = check_and_cast<cPacket *>(msg);
    if (shouldDrop(packet))
        dropPacket(packet);
    else
        sendOut(packet);
}

void AlgorithmicDropperBase::dropPacket(cPacket *packet)
{
    // TODO statistics
    delete packet;
}

void AlgorithmicDropperBase::sendOut(cPacket *packet)
{
    int index = packet->getArrivalGate()->getIndex();
    send(packet, "out", index);
}

int AlgorithmicDropperBase::getLength() const
{
    int len = 0;
    for (const auto & elem : outQueueSet)
        len += (elem)->getLength();
    return len;
}

int AlgorithmicDropperBase::getByteLength() const
{
    int len = 0;
    for (const auto & elem : outQueueSet)
        len += (elem)->getByteLength();
    return len;
}

} // namespace inet


//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/queueing/classifier/PriorityClassifier.h"

namespace inet {
namespace queueing {

Define_Module(PriorityClassifier);

int PriorityClassifier::classifyPacket(Packet *packet)
{
    for (int i = 0; i < (int)consumers.size(); i++) {
        auto outputConsumer = consumers[i];
        if (outputConsumer->canPushSomePacket(outputGates[i]))
            return i;
    }
    return -1;
}

bool PriorityClassifier::canPushSomePacket(cGate *gate) const
{
    for (int i = 0; i < (int)consumers.size(); i++) {
        auto outputConsumer = consumers[i];
        if (outputConsumer->canPushSomePacket(outputGates[i]))
            return true;
    }
    return false;
}

bool PriorityClassifier::canPushPacket(Packet *packet, cGate *gate) const
{
    for (int i = 0; i < (int)consumers.size(); i++) {
        auto consumer = consumers[i];
        if (consumer->canPushPacket(packet, outputGates[i]))
            return true;
    }
    return false;
}

} // namespace queueing
} // namespace inet


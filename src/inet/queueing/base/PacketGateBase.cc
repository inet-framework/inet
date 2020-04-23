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

#include "inet/queueing/base/PacketGateBase.h"

namespace inet {
namespace queueing {

void PacketGateBase::open()
{
    ASSERT(!isOpen_);
    EV_DEBUG << "Opening gate.\n";
    isOpen_ = true;
    if (producer != nullptr)
        producer->handleCanPushPacket(inputGate->getPathStartGate());
    if (collector != nullptr)
        collector->handleCanPullPacket(outputGate->getPathEndGate());
}

void PacketGateBase::close()
{
    ASSERT(isOpen_);
    EV_DEBUG << "Closing gate.\n";
    isOpen_ = false;
}

void PacketGateBase::processPacket(Packet *packet)
{
    EV_INFO << "Passing through packet " << packet->getName() << "." << endl;
}

bool PacketGateBase::canPushSomePacket(cGate *gate) const
{
    return isOpen_ && consumer->canPushSomePacket(outputGate->getPathStartGate());
}

bool PacketGateBase::canPushPacket(Packet *packet, cGate *gate) const
{
    return isOpen_ && consumer->canPushPacket(packet, outputGate->getPathStartGate());
}

bool PacketGateBase::canPullSomePacket(cGate *gate) const
{
    return isOpen_ && provider->canPullSomePacket(inputGate->getPathStartGate());
}

Packet *PacketGateBase::canPullPacket(cGate *gate) const
{
    return isOpen_ ? provider->canPullPacket(inputGate->getPathStartGate()) : nullptr;
}

void PacketGateBase::handleCanPushPacket(cGate *gate)
{
    Enter_Method("handleCanPushPacket");
    if (isOpen_)
        PacketFlowBase::handleCanPushPacket(gate);
}

void PacketGateBase::handleCanPullPacket(cGate *gate)
{
    Enter_Method("handleCanPullPacket");
    if (isOpen_)
        PacketFlowBase::handleCanPullPacket(gate);
}

} // namespace queueing
} // namespace inet


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

#include "inet/common/ModuleAccess.h"
#include "inet/queueing/base/PacketDuplicatorBase.h"
#include "inet/common/Simsignals.h"

namespace inet {
namespace queueing {

void PacketDuplicatorBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        producer = findConnectedModule<IActivePacketSource>(inputGate);
        outputGate = gate("out");
        consumer = findConnectedModule<IPassivePacketSink>(outputGate);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketPushingSupport(inputGate);
        checkPacketPushingSupport(outputGate);
    }
}

void PacketDuplicatorBase::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    int numDuplicates = getNumPacketDuplicates(packet);
    for (int i = 0; i < numDuplicates; i++) {
        EV_INFO << "Forwarding duplicate packet " << packet->getName() << "." << endl;
        auto duplicate = packet->dup();
        pushOrSendPacket(duplicate, outputGate, consumer);
    }
    EV_INFO << "Forwarding original packet " << packet->getName() << "." << endl;
    pushOrSendPacket(packet, outputGate, consumer);
}

void PacketDuplicatorBase::handleCanPushPacket(cGate *gate)
{
    Enter_Method("handleCanPushPacket");
    if (producer != nullptr)
        producer->handleCanPushPacket(inputGate);
}

} // namespace queueing
} // namespace inet


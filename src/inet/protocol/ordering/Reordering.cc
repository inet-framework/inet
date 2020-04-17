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

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/protocol/contract/IProtocol.h"
#include "inet/protocol/ordering/Reordering.h"
#include "inet/protocol/ordering/SequenceNumberHeader_m.h"

namespace inet {

Define_Module(Reordering);

void Reordering::initialize(int stage)
{
    PacketPusherBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        registerService(IProtocol::sequenceNumber, inputGate, inputGate);
        registerProtocol(IProtocol::sequenceNumber, outputGate, outputGate);
    }
}

void Reordering::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    auto header = packet->popAtFront<SequenceNumberHeader>();
    auto sequenceNumber = header->getSequenceNumber();
    packets[sequenceNumber] = packet;
    if (sequenceNumber == expectedSequenceNumber) {
        while (true) {
            auto it = packets.find(expectedSequenceNumber);
            if (it == packets.end())
                break;
            pushOrSendPacket(it->second, outputGate, consumer);
            packets.erase(it);
            expectedSequenceNumber++;
        }
    }
}

} // namespace inet


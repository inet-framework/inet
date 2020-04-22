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
#include "inet/protocol/acknowledgement/AcknowledgeHeader_m.h"
#include "inet/protocol/acknowledgement/ReceiveWithAcknowledge.h"
#include "inet/protocol/contract/IProtocol.h"
#include "inet/protocol/ordering/SequenceNumberHeader_m.h"

namespace inet {

Define_Module(ReceiveWithAcknowledge);

void ReceiveWithAcknowledge::initialize(int stage)
{
    PacketPusherBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        registerService(IProtocol::withAcknowledge, nullptr, inputGate);
        registerProtocol(IProtocol::withAcknowledge, nullptr, outputGate);
    }
}

void ReceiveWithAcknowledge::pushPacket(Packet *dataPacket, cGate *gate)
{
    Enter_Method("pushPacket");
    take(dataPacket);
    auto dataHeader = dataPacket->popAtFront<SequenceNumberHeader>();
    send(dataPacket, "out");
    auto ackHeader = makeShared<AcknowledgeHeader>();
    ackHeader->setSequenceNumber(dataHeader->getSequenceNumber());
    auto ackPacket = new Packet("Ack", ackHeader);
    ackPacket->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&IProtocol::acknowledge);
    send(ackPacket, "ackOut");
}

} // namespace inet


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
#include "inet/common/ProtocolTag_m.h"
#include "inet/protocol/forwarding/HopLimitHeader_m.h"
#include "inet/protocol/forwarding/SendWithHopLimit.h"
#include "inet/protocol/contract/IProtocol.h"

namespace inet {

Define_Module(SendWithHopLimit);

void SendWithHopLimit::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        hopLimit = par("hopLimit");
        registerService(IProtocol::hopLimit, inputGate, inputGate);
        registerProtocol(IProtocol::hopLimit, outputGate, outputGate);
    }
}

void SendWithHopLimit::processPacket(Packet *packet)
{
    packet->removeTagIfPresent<DispatchProtocolReq>();
    auto header = makeShared<HopLimitHeader>();
    header->setHopLimit(hopLimit);
    packet->insertAtFront(header);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&IProtocol::hopLimit);
    pushOrSendPacket(packet, outputGate, consumer);
}

} // namespace inet


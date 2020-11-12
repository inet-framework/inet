//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/protocolelement/forwarding/SendWithHopLimit.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/forwarding/HopLimitHeader_m.h"

namespace inet {

Define_Module(SendWithHopLimit);

void SendWithHopLimit::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        hopLimit = par("hopLimit");
        registerService(AccessoryProtocol::hopLimit, inputGate, nullptr);
        registerProtocol(AccessoryProtocol::hopLimit, outputGate, nullptr);
    }
}

void SendWithHopLimit::processPacket(Packet *packet)
{
    packet->removeTagIfPresent<DispatchProtocolReq>();
    auto header = makeShared<HopLimitHeader>();
    header->setHopLimit(hopLimit);
    packet->insertAtFront(header);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&AccessoryProtocol::hopLimit);
    pushOrSendPacket(packet, outputGate, consumer);
}

} // namespace inet


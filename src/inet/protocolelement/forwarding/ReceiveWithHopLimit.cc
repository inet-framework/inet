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

#include "inet/protocolelement/forwarding/ReceiveWithHopLimit.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/forwarding/HopLimitHeader_m.h"

namespace inet {

Define_Module(ReceiveWithHopLimit);

void ReceiveWithHopLimit::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        registerService(AccessoryProtocol::hopLimit, nullptr, inputGate);
        registerProtocol(AccessoryProtocol::hopLimit, nullptr, outputGate);
    }
}

void ReceiveWithHopLimit::processPacket(Packet *packet)
{
    auto header = packet->popAtFront<HopLimitHeader>();
    packet->popAtFront<HopLimitHeader>();
    packet->addTag<HopLimitInd>()->setHopLimit(header->getHopLimit());
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&AccessoryProtocol::forwarding);
}

bool ReceiveWithHopLimit::matchesPacket(const Packet *packet) const
{
    auto header = packet->peekAtFront<HopLimitHeader>();
    return header->getHopLimit() > 0;
}

} // namespace inet


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
#include "inet/protocol/aggregation/header/SubpacketLengthHeader_m.h"
#include "inet/protocol/aggregation/SubpacketLengthHeaderBasedDeaggregator.h"
#include "inet/protocol/contract/IProtocol.h"

namespace inet {

Define_Module(SubpacketLengthHeaderBasedDeaggregator);

void SubpacketLengthHeaderBasedDeaggregator::initialize(int stage)
{
    DeaggregatorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        registerService(IProtocol::aggregation, inputGate, inputGate);
        registerProtocol(IProtocol::aggregation, outputGate, outputGate);
    }
}

std::vector<Packet *> SubpacketLengthHeaderBasedDeaggregator::deaggregatePacket(Packet *packet)
{
    std::vector<Packet *> subpackets;
    cStringTokenizer tokenizer(packet->getName(), "+");
    while (packet->getDataLength() > b(0)) {
        auto subpacketHeader = packet->popAtFront<SubpacketLengthHeader>();
        auto subpacketData = packet->popAtFront(subpacketHeader->getLengthField());
        auto subpacketName = tokenizer.nextToken();
        auto subpacket = new Packet(subpacketName, subpacketData);
        subpackets.push_back(subpacket);
    }
    return subpackets;
}

} // namespace inet


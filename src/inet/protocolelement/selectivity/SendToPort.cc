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

#include "inet/protocolelement/selectivity/SendToPort.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/selectivity/DestinationPortHeader_m.h"

namespace inet {

Define_Module(SendToPort);

void SendToPort::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        port = par("port");
        registerService(AccessoryProtocol::destinationPort, inputGate, nullptr);
        registerProtocol(AccessoryProtocol::destinationPort, outputGate, nullptr);
    }
}

void SendToPort::processPacket(Packet *packet)
{
    auto header = makeShared<DestinationPortHeader>();
    header->setDestinationPort(port);
    packet->insertAtFront(header);
}

} // namespace inet


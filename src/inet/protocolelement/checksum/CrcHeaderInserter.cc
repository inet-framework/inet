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

#include "inet/protocolelement/checksum/CrcHeaderInserter.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/protocolelement/checksum/header/CrcHeader_m.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"

namespace inet {

Define_Module(CrcHeaderInserter);

void CrcHeaderInserter::initialize(int stage)
{
    CrcInserterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        headerPosition = parseHeaderPosition(par("headerPosition"));
        registerService(AccessoryProtocol::crc, inputGate, nullptr);
        registerProtocol(AccessoryProtocol::crc, outputGate, nullptr);
    }
}

void CrcHeaderInserter::processPacket(Packet *packet)
{
    const auto& header = makeShared<CrcHeader>();
    auto crc = computeCrc(packet, crcMode);
    header->setCrc(crc);
    header->setCrcMode(crcMode);
    insertHeader<CrcHeader>(packet, header, headerPosition);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&AccessoryProtocol::crc);
}

} // namespace inet


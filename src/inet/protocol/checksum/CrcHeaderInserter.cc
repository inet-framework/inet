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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/protocol/checksum/CrcHeaderInserter.h"
#include "inet/protocol/checksum/header/CrcHeader_m.h"
#include "inet/protocol/common/AccessoryProtocol.h"

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


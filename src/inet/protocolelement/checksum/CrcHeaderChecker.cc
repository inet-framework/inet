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

#include "inet/protocolelement/checksum/CrcHeaderChecker.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/protocolelement/checksum/header/CrcHeader_m.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"

namespace inet {

Define_Module(CrcHeaderChecker);

void CrcHeaderChecker::initialize(int stage)
{
    CrcCheckerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        headerPosition = parseHeaderPosition(par("headerPosition"));
        registerService(AccessoryProtocol::crc, nullptr, inputGate);
        registerProtocol(AccessoryProtocol::crc, nullptr, outputGate);
    }
}

void CrcHeaderChecker::processPacket(Packet *packet)
{
    popHeader<CrcHeader>(packet, headerPosition, B(2));
}

bool CrcHeaderChecker::matchesPacket(const Packet *packet) const
{
    const auto& header = peekHeader<CrcHeader>(packet, headerPosition, B(2));
    auto crcMode = header->getCrcMode();
    auto crc = header->getCrc();
    return checkCrc(packet, crcMode, crc);
}

} // namespace inet


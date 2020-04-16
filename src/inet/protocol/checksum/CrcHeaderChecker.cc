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
#include "CrcHeaderChecker.h"
#include "inet/protocol/checksum/header/CrcHeader_m.h"
#include "inet/protocol/contract/IProtocol.h"

namespace inet {

Define_Module(CrcHeaderChecker);

void CrcHeaderChecker::initialize(int stage)
{
    CrcCheckerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        headerPosition = parseHeaderPosition(par("headerPosition"));
        registerService(IProtocol::crc, nullptr, inputGate);
        registerProtocol(IProtocol::crc, nullptr, outputGate);
    }
}

bool CrcHeaderChecker::matchesPacket(const Packet *packet) const
{
    const auto& header = popHeader<CrcHeader>(packet, headerPosition, B(2));
    auto crcMode = header->getCrcMode();
    auto crc = header->getCrc();
    return checkCrc(packet, crcMode, crc);
}

} // namespace inet


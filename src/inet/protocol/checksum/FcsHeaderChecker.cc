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
#include "inet/protocol/checksum/FcsHeaderChecker.h"
#include "inet/protocol/checksum/header/FcsHeader_m.h"
#include "inet/protocol/contract/IProtocol.h"

namespace inet {

Define_Module(FcsHeaderChecker);

void FcsHeaderChecker::initialize(int stage)
{
    FcsCheckerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        headerPosition = parseHeaderPosition(par("headerPosition"));
        registerService(IProtocol::fcs, nullptr, inputGate);
        registerProtocol(IProtocol::fcs, nullptr, outputGate);
    }
}

bool FcsHeaderChecker::matchesPacket(const Packet *packet) const
{
    const auto& header = popHeader<FcsHeader>(packet, headerPosition, B(4));
    auto fcsMode = header->getFcsMode();
    auto fcs = header->getFcs();
    return checkFcs(packet, fcsMode, fcs);
}

} // namespace inet


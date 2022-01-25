//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

#include "inet/linklayer/ieee8021as/GptpPacket_m.h"
#include "inet/queueing/function/PacketClassifierFunction.h"

namespace inet {

static int classifyPacketByGptpDomainNumber(Packet *packet)
{
    const auto& header = packet->peekAtFront<GptpBase>();
    return header->getDomainNumber();
}

Register_Packet_Classifier_Function(GptpDomainNumberClassifier, classifyPacketByGptpDomainNumber);

} // namespace inet


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

#include "inet/protocol/contract/IProtocol.h"

namespace inet {

const Protocol IProtocol::acknowledge("acknowledge", "Acknowledge");
const Protocol IProtocol::aggregation("aggregation", "Aggregation");
const Protocol IProtocol::crc("crc", "CRC");
const Protocol IProtocol::destinationL3Address("destinationl3Address", "Destination L3 address");
const Protocol IProtocol::destinationMacAddress("destinationMacAddress", "Destination MAC address");
const Protocol IProtocol::destinationPort("destinationPort", "Destination port");
const Protocol IProtocol::fcs("fcs", "FCS");
const Protocol IProtocol::forwarding("forwarding", "Forwarding");
const Protocol IProtocol::fragmentation("fragmentation", "Fragmentation");
const Protocol IProtocol::hopLimit("hopLimit", "Hop limit");
const Protocol IProtocol::sequenceNumber("sequenceNumber", "Sequence number");
const Protocol IProtocol::withAcknowledge("withAcknowledge", "With acknowledge");

} // namespace inet

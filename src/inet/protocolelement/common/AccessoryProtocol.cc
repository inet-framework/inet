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

#include "inet/protocolelement/common/AccessoryProtocol.h"

namespace inet {

const Protocol AccessoryProtocol::acknowledge("acknowledge", "Acknowledge");
const Protocol AccessoryProtocol::aggregation("aggregation", "Aggregation");
const Protocol AccessoryProtocol::crc("crc", "CRC");
const Protocol AccessoryProtocol::destinationL3Address("destinationl3Address", "Destination L3 address");
const Protocol AccessoryProtocol::destinationMacAddress("destinationMacAddress", "Destination MAC address");
const Protocol AccessoryProtocol::destinationPort("destinationPort", "Destination port");
const Protocol AccessoryProtocol::fcs("fcs", "FCS");
const Protocol AccessoryProtocol::forwarding("forwarding", "Forwarding");
const Protocol AccessoryProtocol::fragmentation("fragmentation", "Fragmentation");
const Protocol AccessoryProtocol::hopLimit("hopLimit", "Hop limit");
const Protocol AccessoryProtocol::sequenceNumber("sequenceNumber", "Sequence number");
const Protocol AccessoryProtocol::withAcknowledge("withAcknowledge", "With acknowledge");

} // namespace inet

//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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


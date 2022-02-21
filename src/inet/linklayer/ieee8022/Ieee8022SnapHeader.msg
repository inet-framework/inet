//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.linklayer.ieee8022.Ieee8022LlcHeader;

namespace inet;

//
// This chunk represents an IEEE 802.2 LLC header SNAP extension as defined by
// the section 9.5 SNAP of the IEEE Std 802-2014, IEEE Standard for Local and
// Metropolitan Area Networks: Overview and Architecture.
//
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-----------------------------------------------+---------------+
// |                      OUI                      |   PROTOCOLID  |
// +---------------+-------------------------------+---------------+
// |   PROTOCOLID  |
// +---------------+
//
class Ieee8022SnapHeader extends FieldsChunk
{
    chunkLength = B(5);
    int oui = -1;   // organizationally unique identifier (OUI); 0 for protocols that have an EtherType (ARP, IPv4, IPv6, etc.), 3 bytes
    int protocolId = -1; // protocol identifier (PID); stores ~EtherType if orgCode is 0, 2 bytes
}

//
// This chunk represents an IEEE 802.2 LLC header with SNAP extension as defined
// by the section 9.2.4 OUI Extended EtherType of the IEEE Std 802-2014, IEEE
// Standard for Local and Metropolitan Area Networks: Overview and Architecture.
//
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +---------------+---------------+---------------+---------------+
// |      DSAP     |      SSAP     |    CONTROL    |      OUI      |
// +---------------+---------------+---------------+---------------+
// |              OUI              |           PROTOCOLID          |
// +-------------------------------+-------------------------------+
//
class Ieee8022LlcSnapHeader extends Ieee8022LlcHeader
{
    chunkLength = B(8);
    dsap = 0xAA;
    ssap = 0xAA;
    control = 0x03;
    int oui = -1;   // organizationally unique identifier (OUI); 0 for protocols that have an EtherType (ARP, IPv4, IPv6, etc.), 3 bytes
    int protocolId = -1; // protocol identifier (PID); stores ~EtherType if orgCode is 0, 2 bytes
}

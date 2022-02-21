//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.contract.ipv4.Ipv4Address;
import inet.transportlayer.common.CrcMode;

namespace inet::ospf;

enum OspfPacketType
{
    HELLO_PACKET = 1;
    DATABASE_DESCRIPTION_PACKET = 2;
    LINKSTATE_REQUEST_PACKET = 3;
    LINKSTATE_UPDATE_PACKET = 4;
    LINKSTATE_ACKNOWLEDGEMENT_PACKET = 5;
}

//
// Represents an OSPF packet header
//
class OspfPacketBase extends FieldsChunk
{
    chunkLength = B(14);
    short version = -1;     // 1 byte
    OspfPacketType type = static_cast<OspfPacketType>(-1);    // 1 byte
    int packetLengthField;  // 2 bytes

    Ipv4Address routerID;   // 4 bytes
    Ipv4Address areaID;     // 4 bytes

    uint16_t crc = 0;       // 2 bytes
    CrcMode crcMode = CRC_MODE_UNDEFINED;
}

//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.contract.ipv4.Ipv4Address;

namespace inet::vrrp;

//
// VRRPv2 message type (RFC 3768). Only ADVERTISEMENT (1) is defined.
//
enum VrrpType
{
    VRRP_ADVERTISEMENT = 1;
}

//
// VRRPv2 ADVERTISEMENT packet (RFC 3768, section 5.1).
//
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |Version| Type  | Virtual Rtr ID|   Priority    | Count IP Addrs|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |   Auth Type   |   Adver Int   |          Checksum             |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                       IP Address (1..n)                       |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// Note: 'version' and 'type' occupy 4 bits each in the first octet; the
// (deprecated) 8-byte authentication data trailer is not modelled.
//
class VrrpAdvertisement extends FieldsChunk
{
    chunkLength = B(8);              // fixed header; +4 bytes per IP address
    uint8_t version = 2;            // 4 bits on the wire
    uint8_t type = VRRP_ADVERTISEMENT; // 4 bits on the wire
    uint8_t vrid;                  // Virtual Router ID (1..255)
    uint8_t priority;              // 0 = release mastership, 255 = address owner
    uint8_t countIpAddrs;          // number of associated virtual IP addresses
    uint8_t authType = 0;          // deprecated, 0 (No Authentication)
    uint8_t adverInt = 1;          // advertisement interval in seconds
    uint16_t checksum = 0;
    Ipv4Address addresses[];       // associated virtual IP address(es)
}

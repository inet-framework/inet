//
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.linklayer.common.MacAddress;

namespace inet;

class TagBase
{
}

//!TagDefinitionExeample
class MacAddressReq extends TagBase
{
    MacAddress srcAddress;  // may be unspecified
    MacAddress destAddress; // always specified
}
//!End

//!UdpHeaderDefinitionExample
enum CrcMode
{
  CRC_DISABLED = 0; // CRC is not set, serializable
  CRC_DECLARED = 1; // CRC is correct without the value, not serializable
  CRC_COMPUTED = 2; // CRC is potentially incorrect, serializable
}

class UdpHeader extends FieldsChunk
{
  chunkLength = B(8); // UDP header length is always 8 bytes
  int sourcePort = -1; // source port field is undefined by default
  int destinationPort = -1; // destination port field is undefined by default
  B lengthField = B(-1); // length field is undefined by default
  uint16_t crc = 0; // checksum field is 0 by default
  CrcMode crcMode = CRC_DISABLED; // checksum mode is disabled by default
}
//!End

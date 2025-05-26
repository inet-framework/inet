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
enum ChecksumMode
{
  CHECKSUM_DISABLED = 0; // CHECKSUM is not set, serializable
  CHECKSUM_DECLARED = 1; // CHECKSUM is correct without the value, not serializable
  CHECKSUM_COMPUTED = 2; // CHECKSUM is potentially incorrect, serializable
}

class UdpHeader extends FieldsChunk
{
  chunkLength = B(8); // UDP header length is always 8 bytes
  int sourcePort = -1; // source port field is undefined by default
  int destinationPort = -1; // destination port field is undefined by default
  B lengthField = B(-1); // length field is undefined by default
  uint16_t checksum = 0; // checksum field is 0 by default
  ChecksumMode checksumMode = CHECKSUM_DISABLED; // checksum mode is disabled by default
}
//!End

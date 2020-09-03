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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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

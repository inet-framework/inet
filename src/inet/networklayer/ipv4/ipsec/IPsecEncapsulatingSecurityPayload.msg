//
// Copyright (C) 2020 OpenSim Ltd and Marcel Marek
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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet::ipsec;

cplusplus {{
const int ESP_FIXED_HEADER_BYTES = 8;  // length of the SPI and Sequence Number fields
const int ESP_FIXED_PAYLOAD_TRAILER_BYTES = 2; // length of the Pad Length and Next Header fields
}}

//
// IPsec ESP header (RFC 4303).
//
class IPsecEspHeader extends FieldsChunk
{
    chunkLength = B(ESP_FIXED_HEADER_BYTES);
    uint32_t spi; // security parameters index
    uint32_t sequenceNumber;
}

//
// IPsec ESP trailer (RFC 4303).
//
class IPsecEspTrailer extends FieldsChunk
{
    chunkLength = B(ESP_FIXED_PAYLOAD_TRAILER_BYTES);
    uint8_t padLength;
    uint8_t nextHeader; // IP Protocol number defined by IANA (4 = IPv4; 41 IPv6; 6 = TCP)
}


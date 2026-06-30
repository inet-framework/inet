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
const int IP_AH_HEADER_BYTES = 12;  // fixed part of the IPsec AH header (Next Header, Payload Len, Reserved, SPI, Sequence Number); the variable-length ICV is modeled as a separate trailing chunk
}}

//
// IPsec AH header (RFC 4302).
//
class IPsecAuthenticationHeader extends FieldsChunk
{
    chunkLength = B(IP_AH_HEADER_BYTES); // mirror IPsecEspHeader; without this a FieldsChunk has length 0 and insertAtFront() rejects it as empty
    uint8_t nextHeader; // IP Protocol number defined by IANA (4 = IPv4; 41 = IPv6; 6 = TCP)
    uint8_t payloadLength;
    uint16_t reserved;
    uint32_t spi; // security parameters index
    uint32_t sequenceNumber;
    unsigned int icvBytes; // non-existent field, coded in SPI
}

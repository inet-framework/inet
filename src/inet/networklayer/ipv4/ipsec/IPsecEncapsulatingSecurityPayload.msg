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

namespace inet::ipsec;
cplusplus {{
const int IP_ESP_HEADER_BYTES = 8;  // size of IPsec ESP fixed header fields (does not include padding, Next Header, ICV fields)  
}}

//
// IPsec ESP packet (RFC 4303).
//
packet IPsecEncapsulatingSecurityPayload
{
    uint32_t spi; // security parameters index
    uint32_t sequenceNumber;      
    uint8_t nextHeader; // IP Protocol number defined by IANA (4 = IPv4; 41 IPv6; 6 = TCP)
}


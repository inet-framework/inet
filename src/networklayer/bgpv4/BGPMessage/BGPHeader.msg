//
// Copyright (C) 2010 Helene Lageber
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

cplusplus {{
const int BGP_HEADER_OCTETS = 19;
}}

//
// BGPv4 message types
//
enum BGPType
{
    BGP_OPEN = 1;
    BGP_UPDATE = 2;
    BGP_NOTIFICATION = 3;
    BGP_KEEPALIVE = 4;
}

//
// Represents a BGPv4 message header.
//
// Header fields modeled:
//   - Marker: 16 octets (authentication)
//   - Length: 2 octets  (total size of the message)
//   - Type: 1 octet
//
packet BGPHeader
{
    byteLength = BGP_HEADER_OCTETS;
    char type @enum(BGPType);
}


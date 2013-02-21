//
// Copyright (C) 2004, 2009 Andras Varga
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

class noncobject Address;
enum IPProtocolId;

cplusplus {{
#include "Address.h"
#include "IPProtocolId_m.h"
}}

//
// Represents a generic network datagram with generic network addresses. 
//
packet GenericDatagram
{
    @customize(true);
    Address sourceAddress @getter(_getSrcAddr);
    Address destinationAddress @getter(_getDestAddr);
    int transportProtocol enum(IPProtocolId) = IP_PROT_NONE;
    short hopLimit;
}

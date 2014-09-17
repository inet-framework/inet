//
// Copyright (C) 2004 Andras Varga
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
#include "inet/networklayer/common/L3Address.h"
}}

namespace inet;

class noncobject L3Address;

//
// Network level control information for sending/receiving packets.
//
class SimpleNetworkProtocolControlInfo
{
    @customize(true);
    L3Address destinationAddress @getter(_getDestinationAddress);
    L3Address sourceAddress @getter(_getSourceAddress);
    int interfaceId = -1;
    short protocol;
    short hopLimit;
}

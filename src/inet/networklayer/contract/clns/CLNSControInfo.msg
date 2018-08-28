// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//

cplusplus {{
#include "inet/networklayer/contract/clns/CLNSAddress.h"
#include "inet/linklayer/common/MACAddress.h"
}}

namespace inet;

//enum IPProtocolId;

class noncobject CLNSAddress;
class noncobject MACAddress;

//
class CLNSControlInfo
{
    @customize(true);
    
    CLNSAddress destAddr;   // destination IPv4 address
    CLNSAddress srcAddr;    // source IPv4 address
    MACAddress src;
    MACAddress dest;
    int interfaceId = -1; // interface on which the datagram was received, or
                          // should be sent (see ~InterfaceTable)
//    bool multicastLoop;   // if true, sent multicast datagrams will be looped back
//    short protocol @enum(IPProtocolId);  // encapsulated protocol
    
    
    
    short timeToLive;     // maximum hop count
    
}


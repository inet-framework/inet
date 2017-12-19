//
// Copyright (C) 2017 Jan Peter Drees
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
#include "inet/common/INETDefs.h"
#include "inet/linklayer/common/MACAddress.h"
}}

namespace inet;

class noncobject MACAddress;

//
// Frame for ~XMac. Frame size is configurable
// in the MAC layer. 
//
packet XMacFrame
{
    MACAddress src;     // source address
    MACAddress dest;    // destination address
    long sequenceId; // Sequence Number to detect duplicate messages
}


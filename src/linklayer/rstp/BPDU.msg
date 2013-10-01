//
// Copyright (C) 2011 Juan Luis Garrote Molinero
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


class noncobject MACAddress;

cplusplus {{
#include "MACAddress.h"
#include "EtherFrame.h"
}}


//
// BPDU
//
packet BPDUieee8021D
{
    byteLength = 35;
    string displayString = "b=,,,#a5a5ff";
    int rootPriority;			//Proposed root priority. rootPriority+rootMAC define the proposed root id
    MACAddress rootMAC;			//Proposed root MAC
    int cost;					//Cost from root
    int SrcPriority;  			//id priority. Priority + srcMAC define the bridge id
    int PortPriority;			//Priority of the tx port.
    int PortNumber;				//Sender Bridge output port.
    int age;					//Message age  
    bool TC;					//Topology change flag
    bool Ack;					//Acknowledgement flag
}


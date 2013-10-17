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
cplusplus {{
#include "simtime_t.h"
#include "MACAddress.h"
}}

class noncobject MACAddress;

// According to the 802.1D-1998 specification

packet BPDU
{
    unsigned int protocolIdentifier;		  // The Protocol Identifier is encoded in Octets 1 and 2 of the BPDU. It takes the value 0000 0000 0000
											  // 0000, which identifies the Spanning Tree Algorithm and Protocol 
    
    unsigned int protocolVersionIdentifier;   // The Protocol Version Identifier is encoded in Octet 3 of the BPDU. It takes the value 0000 0000.
    
    unsigned int bpduType;					  // The BPDU Type is encoded in Octet 4 of the BPDU. This field shall take the values 0000 0000 and
    										  // 1000 0000. These denote a Configuration BPDU and a Topology Change Notification BPDU, respectively.
    										   
	bool tcaFlag;							  // The Topology Change Acknowledgment flag is encoded in Bit 8 of Octet 5 of the BPDU.
	
	bool tcFlag;							  // The Topology Change flag is encoded in Bit 1 of Octet 5 of the BPDU.
	
	MACAddress rootAddress;				      // The Root Address is encoded in Octets 6 through 13 of the BPDU.
	unsigned int rootPriority;				  // The Root Priority is the second part of the Root Identifier.
	
	unsigned int rootPathCost;				  // The Root Path Cost is encoded in Octets 14 through 17 of the BPDU.
	
	MACAddress bridgeAddress;			  	  // The Bridge Identifier is encoded in Octets 18 through 25 of the BPDU.
	unsigned int bridgePriority;			  // The Bridge Priority is the second part of the Bridge Identifier.
	
	unsigned int portNum;			          // The Port Identifier is encoded in Octets 26 and 27 of the BPDU.
	unsigned int portPriority;				  // The Port Priority is the second part of the Port Identifier.
	
	simtime_t messageAge;				      // The Message Age timer value is encoded in Octets 28 and 29 of the BPDU.
	
	simtime_t maxAge;					      // The Max Age timer value is encoded in Octets 30 and 31 of the BPDU.
											  // The Message Age (Octets 28 and 29) shall be less than Max Age (Octets 30 and 31).
	simtime_t helloTime;					  // The Hello Time timer value is encoded in Octets 32 and 33 of the BPDU.
	
	simtime_t forwardDelay;				      // The Forward Delay timer value is encoded in Octets 34 and 35 of the BPDU.
}

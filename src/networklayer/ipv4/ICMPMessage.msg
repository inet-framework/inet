//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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
#include "inet/common/INETDefs.h"
}}

namespace inet;

//
// Not the real ICMP codes; here it's set up so that codes above 128 are
// informational
//
// TODO: use real ICMP codes
enum ICMPType
{

    ICMP_DESTINATION_UNREACHABLE = 13;
    ICMP_REDIRECT = 5;
    ICMP_TIME_EXCEEDED = 11;
    ICMP_PARAMETER_PROBLEM = 12;

    ICMP_ECHO_REQUEST = 128;
    ICMP_ECHO_REPLY = 129;
    ICMP_TIMESTAMP_REQUEST = 130;
    ICMP_TIMESTAMP_REPLY = 131;
}

cplusplus {{
typedef int ICMPCode;

//
// Codes for type ICMP_DESTINATION_UNREACHABLE
//
enum ICMPDestinationUnreachableCodes
{
    ICMP_DU_NETWORK_UNREACHABLE =                      0,
    ICMP_DU_HOST_UNREACHABLE =                         1,
    ICMP_DU_PROTOCOL_UNREACHABLE =                     2,
    ICMP_DU_PORT_UNREACHABLE =                         3,
    ICMP_DU_FRAGMENTATION_NEEDED =                     4,
    ICMP_DU_SOURCE_ROUTE_FAILED =                      5,
    ICMP_DU_DESTINATION_NETWORK_UNKNOWN =              6,
    ICMP_DU_DESTINATION_HOST_UNKNOWN =                 7,
    ICMP_DU_SOURCE_HOST_ISOLATED =                     8,
    ICMP_DU_NETWORK_PROHIBITED =                       9,
    ICMP_DU_HOST_PROHIBITED =                         10,
    ICMP_DU_NETWORK_UNREACHABLE_FOR_TYPE_OF_SERVICE = 11,
    ICMP_DU_HOST_UNREACHABLE_FOR_TYPE_OF_SERVICE =    12,
    ICMP_DU_COMMUNICATION_PROHIBITED =                13,
    ICMP_DU_HOST_PRECEDENCE_VIOLATION =               14,
    ICMP_DU_PRECEDENCE_CUTOFF_IN_EFFECT =             15,
    ICMP_AODV_QUEUE_FULL =                            16
};

}}

//
// ICMP message class
//
// Note: receiver MUST NOT decapsulate the datagram from ICMP error messages,
// because their contents is conceptually truncated. decapsulate() would result
// in a "length became negative" error; use getEncapsulatedMsg() instead.
//
// TODO: add originator id and sequence number and create an ICMPControlInfo
packet ICMPMessage
{
    int type @enum(ICMPType);
    int code;
}

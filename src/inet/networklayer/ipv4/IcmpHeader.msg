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


import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.transportlayer.common.CrcMode;

namespace inet;

//
// The real ICMP codes
//
enum IcmpType
{

    ICMP_DESTINATION_UNREACHABLE = 3;
    ICMP_SOURCEQUENCH = 4;               // packet lost, slow down
    ICMP_REDIRECT = 5;
    ICMP_ECHO_REQUEST = 8;
    ICMP_ROUTER_ADVERTISEMENT = 9;    // router advertisement
    ICMP_ROUTER_SOLICITATION = 10;    // router solicitation
    ICMP_TIME_EXCEEDED = 11;
    ICMP_PARAMETER_PROBLEM = 12;

    ICMP_ECHO_REPLY = 0;
    ICMP_TIMESTAMP_REQUEST = 13;
    ICMP_TIMESTAMP_REPLY = 14;
    ICMP_INFORMATION_REQUEST = 15;    // information request
    ICMP_INFORMATION_REPLY = 16;    // information reply
    ICMP_MASK_REQUEST = 17;    // address mask request
    ICMP_MASK_REPLY = 18;    // address mask reply
}

enum IcmpRedirectSubcodes
{
    ICMP_REDIRECT_NET                = 0;    // for network
    ICMP_REDIRECT_HOST               = 1;    // for host
    ICMP_REDIRECT_TOSNET             = 2;    // for tos and net
    ICMP_REDIRECT_TOSHOST            = 3;    // for tos and host
}

enum IcmpTimeExceededSubcodes
{
    ICMP_TIMXCEED_INTRANS            = 0;               // ttl==0 in transit
    ICMP_TIMXCEED_REASS              = 1;               // ttl==0 in reass
}

enum IcmpParameterProblemSubcodes
{
    ICMP_PARAMPROB_ERRATPTR          = 0;    // error at param ptr
    ICMP_PARAMPROB_OPTABSENT         = 1;    // req. opt. absent
    ICMP_PARAMPROB_LENGTH            = 2;    // bad length
}

//
// Codes for type ICMP_DESTINATION_UNREACHABLE
//
enum IcmpDestinationUnreachableCodes
{
    ICMP_DU_NETWORK_UNREACHABLE =                      0;
    ICMP_DU_HOST_UNREACHABLE =                         1;
    ICMP_DU_PROTOCOL_UNREACHABLE =                     2;
    ICMP_DU_PORT_UNREACHABLE =                         3;
    ICMP_DU_FRAGMENTATION_NEEDED =                     4;
    ICMP_DU_SOURCE_ROUTE_FAILED =                      5;
    ICMP_DU_DESTINATION_NETWORK_UNKNOWN =              6;
    ICMP_DU_DESTINATION_HOST_UNKNOWN =                 7;
    ICMP_DU_SOURCE_HOST_ISOLATED =                     8;
    ICMP_DU_NETWORK_PROHIBITED =                       9;
    ICMP_DU_HOST_PROHIBITED =                         10;
    ICMP_DU_NETWORK_UNREACHABLE_FOR_TYPE_OF_SERVICE = 11;
    ICMP_DU_HOST_UNREACHABLE_FOR_TYPE_OF_SERVICE =    12;
    ICMP_DU_COMMUNICATION_PROHIBITED =                13;
    ICMP_DU_HOST_PRECEDENCE_VIOLATION =               14;
    ICMP_DU_PRECEDENCE_CUTOFF_IN_EFFECT =             15;
    ICMP_AODV_QUEUE_FULL =                            16;
}

cplusplus {{
typedef int IcmpCode;

inline bool isIcmpInfoType(int type)
{
    return (type == ICMP_ECHO_REPLY
            || type == ICMP_ECHO_REQUEST
            || type == ICMP_ROUTER_ADVERTISEMENT
            || type == ICMP_ROUTER_SOLICITATION
            || type == ICMP_TIMESTAMP_REQUEST
            || type == ICMP_TIMESTAMP_REPLY
            || type == ICMP_INFORMATION_REQUEST
            || type == ICMP_INFORMATION_REPLY
            || type == ICMP_MASK_REQUEST
            || type == ICMP_MASK_REPLY);
}

}}

//
// ICMP message class
//
// Note: receiver MUST NOT decapsulate the datagram from ICMP error messages,
// because their contents is conceptually truncated. decapsulate() would result
// in a "length became negative" error; use getEncapsulatedMsg() instead.
//
// TODO: add originator id and sequence number and create an ICMPControlInfo
class IcmpHeader extends FieldsChunk
{
    chunkLength = B(8);
    IcmpType type;   // 1 byte
    int code = -1;                   // 1 byte
    int chksum = 0;                 // 2 bytes, RFC 1071
    CrcMode crcMode = CRC_MODE_UNDEFINED;
}

class IcmpEchoRequest extends IcmpHeader
{
    type = ICMP_ECHO_REQUEST;
    code = 0;
    int identifier; // sender module id   // 2 bytes
    int seqNumber; // sequence number           // 2 bytes
}

class IcmpEchoReply extends IcmpHeader
{
    type = ICMP_ECHO_REPLY;
    code = 0;
    int identifier; // sender module id   // 2 bytes
    int seqNumber; // sequence number           // 2 bytes
}


//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


class IPDatagram;

cplusplus {{
#include "IPDatagram.h"
}}


enum ICMPType
{
    ICMP_ECHO_REPLY = 0;
    ICMP_DESTINATION_UNREACHABLE = 3;
    ICMP_REDIRECT = 5;
    ICMP_ECHO_REQUEST = 8;
    ICMP_TIME_EXCEEDED = 11;
    ICMP_PARAMETER_PROBLEM = 12;
    ICMP_TIMESTAMP_REQUEST = 13;
    ICMP_TIMESTAMP_REPLY = 14;
};

cplusplus {{
typedef int ICMPCode;
}}


//
// ICMP packet.
//
//  length(): ping messages have 8*20 bits length;
//            error messages have 8*4 bits ICMP header,
//            full IP header of error datagram (+options)
//            + 8*8 bits payload of the error datagram
//            (usually transport layer header)
//
//  author: Jochen Reber
//
//  Interface formats:
//
//  message format for pingIn/pingOut:
//  type: cMessage
//  parameters: "destination_address" -- char[]
//  parList:    "echoInfo" -- ICMPQuery
//                  identifier, sequence number and timestampValid
//                  need to be set by app
//
//  message format to errorOut:
//  type: ICMPMessage
//
class ICMPQuery
{
  fields:
    bool isTimestampValid;
    int identifier;
    int seq;

    // timestamp fields only used in timestamp req/reply
    simtime_t originateTimestamp;
    simtime_t receiveTimestamp;
    simtime_t transmitTimestamp;
};


//
// ICMP message class
//
message ICMPMessage
{
  fields:
    int type enum(ICMPType);
    int code;
    bool isError;
    ICMPQuery query;  // FIXME this is optional: only if isError==false;
                      // encapsulated errorDatagram exists only if isError==true
};


//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//


cplusplus {{
#include "IPv4Address.h"

// base header: version, length, LSR ID, Label space
#define LDP_BASEHEADER_BYTES  10

// FIXME: the length below is just a guess. TBD find lengths for individual TLVs
// making up different LDP packet types, and determine length for each packet type
#define LDP_HEADER_BYTES  (LDP_BASEHEADER_BYTES+20)
}}


class noncobject IPv4Address;

//
// LDP message types
//
enum LDP_MESSAGE_TYPES
{

    NOTIFICATION = 10;
    HELLO = 11;
    INITIALIZATION = 12;
    KEEP_ALIVE = 13;
    ADDRESS = 14;
    ADDRESS_WITHDRAW = 15;
    LABEL_MAPPING = 16;
    LABEL_REQUEST = 17;
    LABEL_WITHDRAW = 18;
    LABEL_RELEASE = 19;
    UNKNOWN = 20;
}

enum LDP_STATUS_TYPES
{

    NO_ROUTE = 13;
}

struct FEC_TLV
{
    IPv4Address addr;
    int length;
}

//
// Base class for LDP packets
//
packet LDPPacket
{
    int type;
    IPv4Address senderAddress;
    IPv4Address receiverAddress;
}


//
// LDP Label Mapping Message
//
packet LDPLabelMapping extends LDPPacket
{
    FEC_TLV fec;
    int label;
}

//
// LDP Label Request Message
//
packet LDPLabelRequest extends LDPPacket
{
    FEC_TLV fec;
}


//
// LDP Hello Message
//
packet LDPHello extends LDPPacket
{
    double holdTime;
    bool tbit;
    bool rbit;
}

//
// LDP notification message
//
packet LDPNotify extends LDPPacket
{
    int status;
    FEC_TLV fec;
}

//
// LDP Ini Message
//
//# FIXME currently unused
packet LDPIni extends LDPPacket
{
    double keepAliveTime;
    bool abit;
    bool dbit;
    int pvLim;
    string receiverLDPIdentifier;
}

//
// LDP Address Message
//
//# FIXME currently unused
packet LDPAddress extends LDPPacket
{
    bool isWithdraw;
    string family;
    string addresses[];
}




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
#include "IPAddress.h"
}}

class noncobject IPAddress;


//
// LDP message types
//
enum LDP_MESSAGE_TYPES
{
    NOTIFICATION=10;
    HELLO=11;
    INITIALIZATION=12;
    KEEP_ALIVE=13;
    ADDRESS=14;
    ADDRESS_WITHDRAW=15;
    LABEL_MAPPING=16;
    LABEL_REQUEST=17;
    LABEL_WITHDRAW=18;
    LABEL_RELEASE=19;
    UNKNOWN=20;
};


//
// Base class for LDP packets
//
message LDPPacket
{
    fields:
        int type;
        IPAddress senderAddress;
        IPAddress receiverAddress;
};


//
// LDP Label Mapping Message
//
message LDPLabelMapping extends LDPPacket
{
    fields:
        int fecId;
        IPAddress fec;
        int label;
};


//
// LDP Label Request Message
//
message LDPLabelRequest extends LDPPacket
{
    fields:
        int fecId;
        IPAddress fec;
};


//
// LDP Hello Message
//
message LDPHello extends LDPPacket
{
     fields:
         double holdTime;
         bool tbit;
         bool rbit;
};


//
// LDP Ini Message
//
//# FIXME currently unused
message LDPIni extends LDPPacket
{
     fields:
         double keepAliveTime;
         bool abit;
         bool dbit;
         int pvLim;
         string receiverLDPIdentifier;
};

//
// LDP Address Message
//
//# FIXME currently unused
message LDPAddress extends LDPPacket
{
     fields:
         bool isWithdraw;
         string family;
         string addresses[];
};




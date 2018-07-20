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


import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.contract.ipv4.Ipv4Address;

namespace inet;

//
// LDP message types
//
enum LdpMessageTypes
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

enum LdpStatusTypes
{

    NO_ROUTE = 13;
}

struct FecTlv
{
    Ipv4Address addr;
    int length;
}

//
// Base class for LDP packets
//
class LdpPacket extends FieldsChunk
{
    int type;
    Ipv4Address senderAddress;
    Ipv4Address receiverAddress;
}


//
// LDP Label Mapping Message
//
class LdpLabelMapping extends LdpPacket
{
    FecTlv fec;
    int label;
}

//
// LDP Label Request Message
//
class LdpLabelRequest extends LdpPacket
{
    FecTlv fec;
}


//
// LDP Hello Message
//
class LdpHello extends LdpPacket
{
    double holdTime;
    bool tbit;
    bool rbit;
}

//
// LDP notification message
//
class LdpNotify extends LdpPacket
{
    int status;
    FecTlv fec;
}

//
// LDP Ini Message
//
//# FIXME currently unused
class LdpIni extends LdpPacket
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
class LdpAddress extends LdpPacket
{
    bool isWithdraw;
    string family;
    string addresses[];
}




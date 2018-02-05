//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2015 Thomas Dreibholz
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.common.packet.Packet;
import inet.networklayer.common.IpProtocolId;
import inet.networklayer.common.L3Address;
import inet.common.TagBase;

cplusplus {{
#include <vector>
#include "inet/transportlayer/contract/sctp/SctpCommand.h"
}}


namespace inet;

cplusplus {{
typedef std::vector<L3Address> AddressVector;
}}

class AddressVector { @existingClass; }

//
// Currently not in use.
//
enum SctpErrorCode
{

}



class SctpSimpleMessage extends cPacket
{
    uint32 dataLen;     //TODO remove it, use set/getDataArraySize() functions only
    uint8_t data[];
    simtime_t creationTime = 0;
    bool encaps = false;
}

class SctpCommandReq extends TagBase
{
    int socketId = -1;   // identifies the socket within the application
    int sid = -1;
    int numMsgs = 1;
    int ssn = -1;
    unsigned short sendUnordered = false;
    double prValue = 0;
    L3Address localAddr;
    L3Address remoteAddr;
    int localPort = -1;
    int remotePort = -1;
    int gate = -1;
    int fd = -1;
}

class SctpSendReq extends SctpCommandReq
{
    unsigned int prMethod;
    bool last;
    unsigned int ppid = 0;
    bool primary = true;
    bool sackNow = false;
}

class SctpOpenReq extends SctpCommandReq
{
    AddressVector localAddresses;
    AddressVector remoteAddresses;
    bool fork = false;               // used only for passive open
    string queueClass = "SctpQueue"; // may be left empty
    string sctpAlgorithmClass;       // may be left empty
    uint32 inboundStreams;
    uint32 outboundStreams;
    bool streamReset;
    bool appLimited;
    int prMethod;
    uint32 numRequests;
    uint32 messagesToPush;
}

class SctpAvailableReq extends SctpCommandReq
{
    int newSocketId;
}

class SctpConnectReq extends SctpCommandReq
{
    AddressVector remoteAddresses;
    int status;
    int inboundStreams;
    int outboundStreams;
}

class SctpRcvReq extends SctpCommandReq
{
    uint32 ppid;
    uint32 tsn;
    uint32 cumTsn;
}

class SctpStatusReq extends SctpCommandReq
{
    int state;
    string stateName;
    L3Address pathId;
    bool active;
}

class SctpResetReq extends SctpCommandReq
{
    unsigned short requestType = 0;
    uint16 instreams = 0;
    uint16 outstreams = 0;
    uint16 streams[];
}

class SctpInfoReq extends SctpCommandReq
{
    int text = 0;
}

class SctpPathInfoReq extends SctpCommandReq
{
    L3Address remoteAddress;
}

class SctpRtoReq extends SctpCommandReq
{
    double rtoInitial;
    double rtoMin;
    double rtoMax;
}

class SctpSendQueueAbatedReq extends SctpCommandReq
{
    uint64 bytesAvailable;
    uint64 bytesQueued;
    uint64 bytesLimit;
    uint64 queuedForStream[];
}

class SctpPathInfo extends cNamedObject
{
    L3Address remoteAddress;
}

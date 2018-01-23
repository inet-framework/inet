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


//
// Control info for SCTP connections. This message is to be set as control info
// (see cMessage::setControlInfo()) on all messages exchanged between ~SCTP and
// application, in both directions. Some commands and indications
// (SCTP_C_OPEN_xxx, SCTP_I_STATUS) use submessagees.
//
// connId identifies the connection locally within the application (internally,
// ~SCTP uses the (app gate index, connId) pair to identify the socket).
// connId is to be chosen by the application in the open command.
//
// @see ~SctpCommandCode, ~SctpStatusInd, ~SctpOpenCommand, ~SctpStatusInfo, SctpMain
//
class SctpCommand extends cPacket
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

//
// Currently not in use.
//
class SctpErrorInfo extends SctpCommand
{
    SctpErrorCode errorCode;
    string messageText;
}


//
// Control info to be used for active or passive ~Sctp open.
//
// localAddr, remoteAddr, localPort, remotePort should be self-explanatory.
// localAddr is optional because ~Sctp can learn it from IP when a packet
// is received from the peer; localPort is optional because ~SCTP supports
// ephemeral ports.
//
// The sendQueueClass, receiveQueueClass and sctpAlgorithmClass fields
// allow per-connection ~Sctp configuration. These fields may contain
// names of messagees submessageed from SctpSendQueue, SctpReceiveQueue
// and SctpAlgorithm, respectively. If not set, module parameters with
// similar names are used.
//
// The fork parameter is used with passive open, and controls what happens
// when an incoming connection is received. With fork=true, it emulates
// the Unix accept(2) syscall semantics: a new connection structure
// is created for the connection (with a new connId, see in ~SctpCommand),
// and the connection structure with the old connId remains listening.
// With fork=false, all the above does not happen: the first connection
// is accepted (with the original connId), and further incoming connections
// will be refused by ~Sctp by sending an RST segment.
//
// @see ~SctpCommandCode, ~Sctp
//
class SctpOpenCommand extends SctpCommand
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


//
// Control info to be used with the SEND command.
//
// @see ~SctpCommandCode, ~Sctp
//
class SctpSendInfo extends SctpCommand
{
    unsigned int prMethod;
    bool last;
    unsigned int ppid = 0;
    bool primary = true;
    bool sackNow = false;
}

//
// Control info to be used when setsockopt changes RtoInfo.
//
// @see ~SctpCommandCode, ~Sctp
//
class SctpRtoInfo extends SctpCommand
{
    double rtoInitial;
    double rtoMin;
    double rtoMax;
}

//
// Control info to be used with the DATA command.
//
// @see ~SctpCommandCode, ~Sctp
//
class SctpRcvInfo extends SctpCommand
{
    uint32 ppid;
    uint32 tsn;
    uint32 cumTsn;
}


//
// Sent with message kind SCTP_I_ESTABLISHED, to let the app know
// about the local and remote IP address and port.
//
// @see ~SctpCommandCode, ~Sctp
//
class SctpConnectInfo extends SctpCommand
{
    AddressVector remoteAddresses;
    int status;
    int inboundStreams;
    int outboundStreams;
}

class SctpAvailableInfo extends SctpCommand
{
    int newSocketId;
}

class SctpAcceptSocketInfo extends SctpCommand
{
}


//
// Sent with message kind SCTP_I_STATUS, in response to command SCTP_C_STATUS.
// For explanation of variables, see RFC 4960 or SctpStateVariables in
// SctpAssociation.h.
//
// @see ~SctpStatusInd, ~SctpCommandCode, ~Sctp
//
class SctpStatusInfo extends SctpCommand
{
    int state;
    string stateName;
    L3Address pathId;
    bool active;
}

class SctpPathInfo extends SctpCommand
{
    L3Address remoteAddress;
}

class SctpResetInfo extends SctpCommand
{
    unsigned short requestType = 0;
    uint16 instreams = 0;
    uint16 outstreams = 0;
    uint16 streams[];
}

class SctpInfo extends SctpCommand
{
    int text = 0;
}

class SctpSendQueueAbated extends SctpCommand
{
    uint64 bytesAvailable;
    uint64 bytesQueued;
    uint64 bytesLimit;
    uint64 queuedForStream[];
}

class SctpSimpleMessage extends cPacket
{
    uint32 dataLen;     //TODO remove it, use set/getDataArraySize() functions only
    char data[];
    simtime_t creationTime = 0;
    bool encaps = false;
}


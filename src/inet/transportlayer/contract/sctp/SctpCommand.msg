//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2015 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.common.packet.Packet;
import inet.networklayer.common.IpProtocolId;
import inet.networklayer.common.L3Address;
import inet.common.TagBase;

cplusplus {{
#include <vector>
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
// SCTP command codes, sent by the application to SCTP. These constants
// should be set as message kind on a message sent to the SCTP entity.
//
// @see SctpCommand, SctpOpenCommand, Sctp
//
enum SctpCommandCode {
    SCTP_C_ASSOCIATE         = 1; // active open (must carry SctpOpenCommand)
    SCTP_C_OPEN_PASSIVE      = 2; // passive open (must carry SctpOpenCommand)
    SCTP_C_SEND              = 3; // send data (set on data packet)
    SCTP_C_CLOSE             = 5; // shutdown the association
    SCTP_C_ABORT             = 6; // abort connection
    SCTP_C_STATUS            = 7; // request status info (SCTP_I_STATUS) from Sctp
    SCTP_C_RECEIVE           = 8; // data receive request
    SCTP_C_SEND_ORDERED      = 9; // send data ordered
    SCTP_C_SEND_UNORDERED    = 10; // send data unordered
    SCTP_C_PRIMARY           = 11; // set primary path
    SCTP_C_QUEUE_BYTES_LIMIT = 12; // set send queue limit (in bytes)
    SCTP_C_QUEUE_MSGS_LIMIT  = 13; // set send queue limit (in messages)
    SCTP_C_SHUTDOWN          = 14;
    SCTP_C_NO_OUTSTANDING    = 15;
    SCTP_C_STREAM_RESET      = 16; // send StreamResetChunk
    SCTP_C_RESET_ASSOC       = 17;
    SCTP_C_ADD_STREAMS       = 18;
    SCTP_C_NAT_INFO          = 19;
    SCTP_C_SEND_ASCONF       = 20;
    SCTP_C_SET_STREAM_PRIO   = 21;
    SCTP_C_GETSOCKETOPTIONS  = 22;
    SCTP_C_ACCEPT            = 23;
    SCTP_C_SET_RTO_INFO      = 24;
    SCTP_C_ACCEPT_SOCKET_ID  = 25;
    SCTP_C_DESTROY           = 26; // destroy socket
};

//
// SCTP indications, sent by SCTP to the application. SCTP will set these
// constants as message kind on messages it sends to the application.
//
// @see SctpCommand, SctpStatusInfo, Sctp
//
enum SctpStatusInd {
    SCTP_I_DATA                  = 1; // data packet (set on data packet)
    SCTP_I_DATA_NOTIFICATION     = 2; // data arrived notification
    SCTP_I_ESTABLISHED           = 3; // connection established
    SCTP_I_PEER_CLOSED           = 4; // FIN received from remote SCTP
    SCTP_I_CLOSED                = 5; // connection closed normally (via FIN exchange)
    SCTP_I_CONNECTION_REFUSED    = 6; // connection refused
    SCTP_I_CONNECTION_RESET      = 7; // connection reset
    SCTP_I_TIMED_OUT             = 8; // conn-estab timer went off, or max retransm. count reached
    SCTP_I_STATUS                = 9; // status info (will carry SctpStatusInfo)
    SCTP_I_ABORT                 = 10; // association was aborted by the peer
    SCTP_I_CONN_LOST             = 11; // association had too many retransmissions FIXME
    SCTP_I_SEND_MSG              = 12;
    SCTP_I_SHUTDOWN_RECEIVED     = 13;
    SCTP_I_SENDQUEUE_FULL        = 14;
    SCTP_I_SENDQUEUE_ABATED      = 15;
    SCTP_I_ABANDONED             = 16;
    SCTP_I_SEND_STREAMS_RESETTED = 17;
    SCTP_I_RCV_STREAMS_RESETTED  = 18;
    SCTP_I_RESET_REQUEST_FAILED  = 19;
    SCTP_I_ADDRESS_ADDED         = 20; // used for AddIP and multihomed NAT
    SCTP_I_SENDSOCKETOPTIONS     = 21;
    SCTP_I_AVAILABLE             = 22;
};

enum SctpFlags {
    COMPLETE_MESG_UNORDERED = 1;
    COMPLETE_MESG_ORDERED   = 0;
};

class SctpSimpleMessage extends cPacket
{
    uint32_t dataLen;     //TODO remove it, use set/getDataArraySize() functions only
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
    uint32_t inboundStreams;
    uint32_t outboundStreams;
    bool streamReset;
    bool appLimited;
    int prMethod;
    uint32_t numRequests;
    uint32_t messagesToPush;
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
    uint32_t ppid;
    uint32_t tsn;
    uint32_t cumTsn;
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
    uint16_t instreams = 0;
    uint16_t outstreams = 0;
    uint16_t streams[];
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
    uint64_t bytesAvailable;
    uint64_t bytesQueued;
    uint64_t bytesLimit;
    uint64_t queuedForStream[];
}

class SctpPathInfo extends cNamedObject
{
    L3Address remoteAddress;
}

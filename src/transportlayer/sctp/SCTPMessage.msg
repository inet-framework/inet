//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

cplusplus {{
#include "inet/networklayer/common/L3Address.h"
}}

namespace inet::sctp;

cplusplus {{
typedef cPacket *cPacketPtr;
}}


struct cPacketPtr;


class noncobject L3Address;

message SCTPMessage extends cPacket
{
    @customize(true);
    // Source Port
    uint16 srcPort;
    // Destination Port
    uint16 destPort;
    // Verification Tag
    uint32 tag;
    bool checksumOk;
    abstract cPacketPtr chunks[];
}


message SCTPChunk extends cPacket
{
    @customize(false);
    // Chunk Type
    uint8 chunkType;
}

message SCTPInitChunk extends SCTPChunk
{
    // Initiate Tag
    uint32 initTag;
    // Advertised Receiver Window
    uint32 a_rwnd;
    // Number of Outbound Streams
    uint16 noOutStreams;
    // Number of Inbound Streams
    uint16 noInStreams;
    // Initial TSN
    uint32 initTSN;
    bool forwardTsn;
    L3Address addresses[];
    uint8 unrecognizedParameters[]; //Will be filled by the Parser, if unrecognized Parameters arrive.
    uint32 msg_rwnd;
    uint16 chunkTypes[]; //Chunk list parameter, i.e. for SCTP AUTH
    uint16 sepChunks[]; //Supported extensions parameter, i.e. for StreamReset
    uint16 hmacTypes[]; //HMAC Identifier for HMAC algorithm parameter
    uint8 random[]; //Random Parameter for SCTP AUTH
}

message SCTPInitAckChunk extends SCTPChunk
{
    // Initiate Tag
    uint32 initTag;
    // Advertised Receiver Window
    uint32 a_rwnd;
    // Number of Outbound Streams
    uint16 noOutStreams;
    // Number of Inbound Streams
    uint16 noInStreams;
    // Initial TSN
    uint32 initTSN;
    bool forwardTsn;
    L3Address addresses[];
    char cookie[];
    uint8 unrecognizedParameters[];
    cPacketPtr stateCookie;
    uint32 msg_rwnd;
    uint8 random[]; //Random Number
    uint16 chunkTypes[];
    uint16 sepChunks[]; //Supported extensions parameter, i.e. for StreamReset
    uint16 hmacTypes[];
}



message SCTPCookieEchoChunk extends SCTPChunk
{
    char cookie[];
    cPacketPtr stateCookie;
    uint8 unrecognizedParameters[]; //for the serializer to send an Error chunk bundled with a Cookie-Echo
}

message SCTPCookie extends cPacket
{
    simtime_t creationTime;
    uint32 localTag;
    uint32 peerTag;
    uint8 localTieTag[];
    uint8 peerTieTag[];
}

message SCTPCookieAckChunk extends SCTPChunk
{
}

message SCTPDataMsg extends cPacket
{
    // Chunk Flags
    bool eBit = 0;
    bool bBit = 0;
    simtime_t enqueuingTime;
    simtime_t expiryTime;
    uint32 ppid;
    L3Address initialDestination;
    uint32 msgNum;
    uint16 sid;
    bool ordered;
    uint32 booksize;
    uint32 rtx;
    bool fragment;
    uint32 prMethod;
    uint32 priority;
    bool strReset;
    bool sackNow;
}

message SCTPDataChunk extends SCTPChunk
{
    // Chunk Flags
    bool eBit = 0;
    bool bBit = 0;
    bool uBit = 0;
    bool iBit = 0;
    // Transmission Sequence Number
    uint32 tsn;
    // Stream identifier
    uint16 sid;
    // Stream Sequence Number
    uint16 ssn;
    // Payload Protocol Identifier
    uint32 ppid;
    simtime_t enqueuingTime;
    simtime_t firstSendTime;
}

message SCTPForwardTsnChunk extends SCTPChunk
{
    uint32 newCumTsn;
    uint16 sid[];
    short ssn[]; // set -1 in case of unordered delivery
}

message SCTPSackChunk extends SCTPChunk
{
    // Cumulative TSN Ack
    uint32 cumTsnAck;
    // Advertised Receiver Window Credit
    uint32 a_rwnd;
    // Number of Gap Ack Blocks
    uint16 numGaps;
    // Number of Non-Revokable Gap Ack Blocks
    uint16 numNrGaps;
    // Number of Duplicate TSNs
    uint16 numDupTsns;
    // Start and End of Gap Ack Blocks
    bool   isNrSack;
    uint32 gapStart[];
    uint32 gapStop[];
    uint32 dupTsns[];
    uint32 sackSeqNum = 0;
    uint32 nrGapStart[];
    uint32 nrGapStop[];
    uint32 msg_rwnd;
    uint8  dacPacketsRcvd = 0;
    bool   nrSubtractRGaps = false;
}

message SCTPAbortChunk extends SCTPChunk
{
    // Chunk Flags
    bool T_Bit = 0;
}

message SCTPHeartbeatChunk extends SCTPChunk
{
    L3Address remoteAddr;
    simtime_t timeField;
    char info[];
}

message SCTPHeartbeatAckChunk extends SCTPChunk
{
    L3Address remoteAddr;
    simtime_t timeField;
    char info[];
}

message SCTPShutdownChunk extends SCTPChunk
{
    uint32 cumTsnAck;
}

message SCTPShutdownAckChunk extends SCTPChunk
{
}

message SCTPShutdownCompleteChunk extends SCTPChunk
{
    bool TBit = 0;
}

message SCTPErrorChunk extends SCTPChunk
{
    @customize(true);
    bool TBit = 0;
    bool MBit = 0;
    abstract cPacketPtr parameters[];
}

message SCTPParameter extends cPacket
{
    uint16 parameterType;
}

message SCTPPacketDropChunk extends SCTPChunk
{
    bool cFlag;
    bool tFlag;
    bool bFlag;
    bool mFlag;
    uint32 maxRwnd;
    uint32 queuedData;
    uint16 truncLength;
}

message SCTPStreamResetChunk extends SCTPChunk
{
    @customize(true);
    abstract cPacketPtr parameters[];
}

message SCTPOutgoingSSNResetRequestParameter extends SCTPParameter
{
    uint32 srReqSn; //Stream Reset Request Sequence Number: initialized with the initial TSN, then incremented
    uint32 srResSn; //Stream Reset Response Sequence Number
    uint32 lastTsn; //Senders last assigned TSN
    uint16 streamNumbers[];
}

message SCTPIncomingSSNResetRequestParameter extends SCTPParameter
{
    uint32 srReqSn; //Stream Reset Request Sequence Number
    uint16 streamNumbers[];
}

message SCTPSSNTSNResetRequestParameter extends SCTPParameter
{
    uint32 srReqSn; //Stream Reset Request Sequence Number
}

message SCTPStreamResetResponseParameter extends SCTPParameter
{
    uint32 srResSn; //Stream Reset Response Sequence Number
    uint32 result;
    uint32 sendersNextTsn = 0;
    uint32 receiversNextTsn = 0;
}

message SCTPResetTimer extends cPacket
{
    uint32 inSN; //Stream Reset Response Sequence Number for Incoming Reset Request
    bool inAcked;   // false, when timer is running for the Reset Response Sequence Number, true otherwise
    uint32 outSN; //Stream Reset Response Sequence Number for Outgoing Reset Request
    bool outAcked;
}

message SCTPAuthenticationChunk extends SCTPChunk
{
    uint16 sharedKey;
    uint16 hMacIdentifier;
    bool hMacOk;
    uint32 HMAC[];
}

message SCTPAsconfChunk extends SCTPChunk
{
    @customize(true);
    uint32 serialNumber;
    L3Address addressParam;
    uint32 peerVTag; //for NAT
    abstract cPacketPtr asconfParams[];
}

message SCTPAsconfAckChunk extends SCTPChunk
{
    @customize(true);
    uint32 serialNumber;
    abstract cPacketPtr asconfResponse[];
}

message SCTPAddIPParameter extends SCTPParameter
{
    uint32 requestCorrelationId; //assigned by the sender to identify each request parameter
    L3Address addressParam;
}

message SCTPDeleteIPParameter extends SCTPParameter
{
    uint32 requestCorrelationId; //assigned by the sender to identify each request parameter
    L3Address addressParam;
}

message SCTPSetPrimaryIPParameter extends SCTPParameter
{
    uint32 requestCorrelationId; //assigned by the sender to identify each request parameter
    L3Address addressParam;
}

message SCTPSupportedExtensionsParameter extends SCTPParameter
{
    uint16 chunkTypes[];
}

message SCTPErrorCauseParameter extends SCTPParameter
{
    uint32 responseCorrelationId;
    uint32 errorCauseType;
}

message SCTPSimpleErrorCauseParameter extends SCTPParameter
{
    uint16 value;
}

message SCTPSuccessIndication extends SCTPParameter
{
    uint32 responseCorrelationId;
}

message NatMessage extends cPacket
{
    bool        multi;
    uint16      peer1;
    L3Address     peer1Addresses[];
    uint32      portPeer1;
    uint16      peer2;
    L3Address     peer2Addresses[];
    uint32      portPeer2;
}

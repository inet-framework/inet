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

import inet.common.INETDefs;
import inet.networklayer.common.L3Address;
import inet.transportlayer.contract.TransportHeaderBase;
import inet.transportlayer.common.CrcMode;
import inet.common.packet.Message;

namespace inet::sctp;

class SctpHeader extends TransportHeaderBase
{
    @customize;
    // Source Port
    uint16 srcPort;
    // Destination Port
    uint16 destPort;
    // Verification Tag
    uint32 vTag;
    bool checksumOk;
    uint32_t crc = 0;
    CrcMode crcMode = CRC_MODE_UNDEFINED;
    unsigned short headerLength = 12;
    abstract SctpChunk *sctpChunks[];
}


class SctpChunk extends cPacket
{
    // Chunk Type
    uint8 sctpChunkType;
    uint32 flags;
    unsigned short length = 0;
    char chunkName[];
}

class SctpInitChunk extends SctpChunk
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
    uint32 initTsn;
    bool forwardTsn;
    bool ipv4Supported;
    bool ipv6Supported;
    L3Address addresses[];
    uint8 unrecognizedParameters[]; //Will be filled by the Parser, if unrecognized Parameters arrive.
    uint32 msg_rwnd;
    uint16 sctpChunkTypes[]; //Chunk list parameter, i.e. for SCTP AUTH
    uint16 sepChunks[]; //Supported extensions parameter, i.e. for StreamReset
    uint16 hmacTypes[]; //HMAC Identifier for HMAC algorithm parameter
    uint8 random[]; //Random Parameter for SCTP AUTH
}

class SctpInitAckChunk extends SctpChunk
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
    uint32 initTsn;
    bool forwardTsn;
    bool ipv4Supported;
    bool ipv6Supported;
    L3Address addresses[];
    char cookie[];
    uint8 unrecognizedParameters[];
    SctpCookie *stateCookie = nullptr;
    uint32 msg_rwnd;
    uint8 random[]; //Random Number
    uint16 sctpChunkTypes[];
    uint16 sepChunks[]; //Supported extensions parameter, i.e. for StreamReset
    uint16 hmacTypes[];
}



class SctpCookieEchoChunk extends SctpChunk
{
    char cookie[];
    SctpCookie *stateCookie = nullptr;
    uint8 unrecognizedParameters[]; //for the serializer to send an Error chunk bundled with a Cookie-Echo
}

class SctpCookie extends cNamedObject
{
    simtime_t creationTime;
    uint32 localTag;
    uint32 peerTag;
    uint8 localTieTag[];
    uint8 peerTieTag[];
    uint32 length;
}

class SctpCookieAckChunk extends SctpChunk
{
}

class SctpDataMsg extends cPacket
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

class SctpDataChunk extends SctpChunk
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

class SctpForwardTsnChunk extends SctpChunk
{
    uint32 newCumTsn;
    uint16 sid[];
    short ssn[]; // set -1 in case of unordered delivery
}

class SctpSackChunk extends SctpChunk
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

class SctpAbortChunk extends SctpChunk
{
    // Chunk Flags
    bool T_Bit = 0;
}

class SctpHeartbeatChunk extends SctpChunk
{
    L3Address remoteAddr;
    simtime_t timeField;
    char info[];
}

class SctpHeartbeatAckChunk extends SctpChunk
{
    L3Address remoteAddr;
    simtime_t timeField;
    char info[];
}

class SctpShutdownChunk extends SctpChunk
{
    uint32 cumTsnAck;
}

class SctpShutdownAckChunk extends SctpChunk
{
}

class SctpShutdownCompleteChunk extends SctpChunk
{
    bool TBit = 0;
}

class SctpErrorChunk extends SctpChunk
{
    @customize;
    bool TBit = 0;
    bool MBit = 0;
    abstract SctpParameter *parameters[];
}

class SctpParameter extends cPacket
{
    uint16 parameterType;
    char name[];
}

class SctpPacketDropChunk extends SctpChunk
{
    bool cFlag;
    bool tFlag;
    bool bFlag;
    bool mFlag;
    uint32 maxRwnd;
    uint32 queuedData;
    uint16 truncLength;
}

class SctpStreamResetChunk extends SctpChunk
{
    @customize;
    abstract SctpParameter *parameters[];
}

class SctpOutgoingSsnResetRequestParameter extends SctpParameter
{
    uint32 srReqSn; //Stream Reset Request Sequence Number: initialized with the initial TSN, then incremented
    uint32 srResSn; //Stream Reset Response Sequence Number
    uint32 lastTsn; //Senders last assigned TSN
    uint16 streamNumbers[];
}

class SctpIncomingSsnResetRequestParameter extends SctpParameter
{
    @customize;
    uint32 srReqSn; //Stream Reset Request Sequence Number
    uint16 streamNumbers[];
}

class SctpSsnTsnResetRequestParameter extends SctpParameter
{
    uint32 srReqSn; //Stream Reset Request Sequence Number
}

class SctpStreamResetResponseParameter extends SctpParameter
{
    uint32 srResSn; //Stream Reset Response Sequence Number
    uint32 result;
    uint32 sendersNextTsn = 0;
    uint32 receiversNextTsn = 0;
}

class SctpAddStreamsRequestParameter extends SctpParameter
{
    uint32 srReqSn; //Stream Reset Request Sequence Number
    uint16 numberOfStreams;
    uint16 reserved = 0;
}

class SctpResetTimer extends cPacket
{
    uint32 inSN; //Stream Reset Response Sequence Number for Incoming Reset Request
    bool inAcked;   // false, when timer is running for the Reset Response Sequence Number, true otherwise
    uint32 outSN; //Stream Reset Response Sequence Number for Outgoing Reset Request
    bool outAcked;
}

class SctpAuthenticationChunk extends SctpChunk
{
    uint16 sharedKey;
    uint16 hMacIdentifier;
    bool hMacOk;
    uint32 HMAC[];
}

class SctpAsconfChunk extends SctpChunk
{
    @customize;
    uint32 serialNumber;
    L3Address addressParam;
    uint32 peerVTag; //for NAT
    abstract SctpParameter *asconfParams[];
}

class SctpAsconfAckChunk extends SctpChunk
{
    @customize;
    uint32 serialNumber;
    abstract SctpParameter *asconfResponse[];
}

class SctpAddIPParameter extends SctpParameter
{
    uint32 requestCorrelationId; //assigned by the sender to identify each request parameter
    L3Address addressParam;
}

class SctpDeleteIPParameter extends SctpParameter
{
    uint32 requestCorrelationId; //assigned by the sender to identify each request parameter
    L3Address addressParam;
}

class SctpSetPrimaryIPParameter extends SctpParameter
{
    uint32 requestCorrelationId; //assigned by the sender to identify each request parameter
    L3Address addressParam;
}

class SctpSupportedExtensionsParameter extends SctpParameter
{
    uint16 sctpChunkTypes[];
}

class SctpErrorCauseParameter extends SctpParameter
{
    uint32 responseCorrelationId;
    uint32 errorCauseType;
}

class SctpSimpleErrorCauseParameter extends SctpParameter
{
    uint16 value;
}

class SctpSuccessIndication extends SctpParameter
{
    uint32 responseCorrelationId;
}

class NatMessage extends cMessage
{
    bool        multi;
    uint16      peer1;
    L3Address     peer1Addresses[];
    uint32      portPeer1;
    uint16      peer2;
    L3Address     peer2Addresses[];
    uint32      portPeer2;
}

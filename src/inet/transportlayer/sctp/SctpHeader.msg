//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
    uint16_t srcPort;
    // Destination Port
    uint16_t destPort;
    // Verification Tag
    uint32_t vTag;
    bool checksumOk;
    uint32_t crc = 0;
    CrcMode crcMode = CRC_MODE_UNDEFINED;
    unsigned short headerLength = 12;
    abstract SctpChunk *sctpChunks[];
}

class SctpChunk extends cPacket
{
    @packetData;
    // Chunk Type
    uint8_t sctpChunkType;
    uint32_t flags;
    unsigned short length = 0;
    char chunkName[];
}

class SctpInitChunk extends SctpChunk
{
    // Initiate Tag
    uint32_t initTag;
    // Advertised Receiver Window
    uint32_t a_rwnd;
    // Number of Outbound Streams
    uint16_t noOutStreams;
    // Number of Inbound Streams
    uint16_t noInStreams;
    // Initial TSN
    uint32_t initTsn;
    bool forwardTsn;
    bool ipv4Supported;
    bool ipv6Supported;
    L3Address addresses[];
    uint8_t unrecognizedParameters[]; //Will be filled by the Parser, if unrecognized Parameters arrive.
    uint32_t msg_rwnd;
    uint16_t sctpChunkTypes[]; //Chunk list parameter, i.e. for SCTP AUTH
    uint16_t sepChunks[]; //Supported extensions parameter, i.e. for StreamReset
    uint16_t hmacTypes[]; //HMAC Identifier for HMAC algorithm parameter
    uint8_t random[]; //Random Parameter for SCTP AUTH
}

class SctpInitAckChunk extends SctpChunk
{
    // Initiate Tag
    uint32_t initTag;
    // Advertised Receiver Window
    uint32_t a_rwnd;
    // Number of Outbound Streams
    uint16_t noOutStreams;
    // Number of Inbound Streams
    uint16_t noInStreams;
    // Initial TSN
    uint32_t initTsn;
    bool forwardTsn;
    bool ipv4Supported;
    bool ipv6Supported;
    L3Address addresses[];
    char cookie[];
    uint8_t unrecognizedParameters[];
    SctpCookie *stateCookie;
    uint32_t msg_rwnd;
    uint8_t random[]; //Random Number
    uint16_t sctpChunkTypes[];
    uint16_t sepChunks[]; //Supported extensions parameter, i.e. for StreamReset
    uint16_t hmacTypes[];
}

class SctpCookieEchoChunk extends SctpChunk
{
    char cookie[];
    SctpCookie *stateCookie;
    uint8_t unrecognizedParameters[]; //for the serializer to send an Error chunk bundled with a Cookie-Echo
}

class SctpCookie extends cNamedObject
{
    simtime_t creationTime;
    uint32_t localTag;
    uint32_t peerTag;
    uint8_t localTieTag[];
    uint8_t peerTieTag[];
    uint32_t length;
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
    uint32_t ppid;
    L3Address initialDestination;
    uint32_t msgNum;
    uint16_t sid;
    bool ordered;
    uint32_t booksize;
    uint32_t rtx;
    bool fragment;
    uint32_t prMethod;
    uint32_t priority;
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
    uint32_t tsn;
    // Stream identifier
    uint16_t sid;
    // Stream Sequence Number
    uint16_t ssn;
    // Payload Protocol Identifier
    uint32_t ppid;
    simtime_t enqueuingTime;
    simtime_t firstSendTime;
}

class SctpForwardTsnChunk extends SctpChunk
{
    uint32_t newCumTsn;
    uint16_t sid[];
    short ssn[]; // set -1 in case of unordered delivery
}

class SctpSackChunk extends SctpChunk
{
    // Cumulative TSN Ack
    uint32_t cumTsnAck;
    // Advertised Receiver Window Credit
    uint32_t a_rwnd;
    // Number of Gap Ack Blocks
    uint16_t numGaps;
    // Number of Non-Revokable Gap Ack Blocks
    uint16_t numNrGaps;
    // Number of Duplicate TSNs
    uint16_t numDupTsns;
    // Start and End of Gap Ack Blocks
    bool   isNrSack;
    uint32_t gapStart[];
    uint32_t gapStop[];
    uint32_t dupTsns[];
    uint32_t sackSeqNum = 0;
    uint32_t nrGapStart[];
    uint32_t nrGapStop[];
    uint32_t msg_rwnd;
    uint8_t  dacPacketsRcvd = 0;
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
    uint32_t cumTsnAck;
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
    @packetData;
    uint16_t parameterType;
}

class SctpPacketDropChunk extends SctpChunk
{
    bool cFlag;
    bool tFlag;
    bool bFlag;
    bool mFlag;
    uint32_t maxRwnd;
    uint32_t queuedData;
    uint16_t truncLength;
}

class SctpStreamResetChunk extends SctpChunk
{
    @customize;
    abstract SctpParameter *parameters[];
}

class SctpOutgoingSsnResetRequestParameter extends SctpParameter
{
    uint32_t srReqSn; //Stream Reset Request Sequence Number: initialized with the initial TSN, then incremented
    uint32_t srResSn; //Stream Reset Response Sequence Number
    uint32_t lastTsn; //Senders last assigned TSN
    uint16_t streamNumbers[];
}

class SctpIncomingSsnResetRequestParameter extends SctpParameter
{
    @customize;
    uint32_t srReqSn; //Stream Reset Request Sequence Number
    uint16_t streamNumbers[];
}

class SctpSsnTsnResetRequestParameter extends SctpParameter
{
    uint32_t srReqSn; //Stream Reset Request Sequence Number
}

class SctpStreamResetResponseParameter extends SctpParameter
{
    uint32_t srResSn; //Stream Reset Response Sequence Number
    uint32_t result;
    uint32_t sendersNextTsn = 0;
    uint32_t receiversNextTsn = 0;
}

class SctpAddStreamsRequestParameter extends SctpParameter
{
    uint32_t srReqSn; //Stream Reset Request Sequence Number
    uint16_t numberOfStreams;
    uint16_t reserved = 0;
}

class SctpResetTimer extends cPacket
{
    uint32_t inSN; //Stream Reset Response Sequence Number for Incoming Reset Request
    bool inAcked;   // false, when timer is running for the Reset Response Sequence Number, true otherwise
    uint32_t outSN; //Stream Reset Response Sequence Number for Outgoing Reset Request
    bool outAcked;
}

class SctpAuthenticationChunk extends SctpChunk
{
    uint16_t sharedKey;
    uint16_t hMacIdentifier;
    bool hMacOk;
    uint32_t HMAC[];
}

class SctpAsconfChunk extends SctpChunk
{
    @customize;
    uint32_t serialNumber;
    L3Address addressParam;
    uint32_t peerVTag; //for NAT
    abstract SctpParameter *asconfParams[];
}

class SctpAsconfAckChunk extends SctpChunk
{
    @customize;
    uint32_t serialNumber;
    abstract SctpParameter *asconfResponse[];
}

class SctpAddIPParameter extends SctpParameter
{
    uint32_t requestCorrelationId; //assigned by the sender to identify each request parameter
    L3Address addressParam;
}

class SctpDeleteIPParameter extends SctpParameter
{
    uint32_t requestCorrelationId; //assigned by the sender to identify each request parameter
    L3Address addressParam;
}

class SctpSetPrimaryIPParameter extends SctpParameter
{
    uint32_t requestCorrelationId; //assigned by the sender to identify each request parameter
    L3Address addressParam;
}

class SctpSupportedExtensionsParameter extends SctpParameter
{
    uint16_t sctpChunkTypes[];
}

class SctpErrorCauseParameter extends SctpParameter
{
    uint32_t responseCorrelationId;
    uint32_t errorCauseType;
}

class SctpSimpleErrorCauseParameter extends SctpParameter
{
    uint16_t value;
}

class SctpSuccessIndication extends SctpParameter
{
    uint32_t responseCorrelationId;
}

class NatMessage extends cMessage
{
    bool        multi;
    uint16_t      peer1;
    L3Address     peer1Addresses[];
    uint32_t      portPeer1;
    uint16_t      peer2;
    L3Address     peer2Addresses[];
    uint32_t      portPeer2;
}

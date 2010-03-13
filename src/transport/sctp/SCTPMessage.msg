//
// Copyright (C) 2008 Irene Ruengeler
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
#include "IPvXAddress.h"
}}



cplusplus {{
typedef cPacket *cPacketPtr;
}}


struct cPacketPtr;


class noncobject IPvXAddress;

message SCTPSimpleMessage extends cPacket
{
     uint32 dataLen;
     char data[];       //cd
     simtime_t creationTime = 0;
     bool encaps = false;
}

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
     IPvXAddress addresses[];
     uint8 unrecognizedParameters[]; //Will be filled by the Parser, if unrecognized Parameters arrive.
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
     IPvXAddress addresses[];
     char cookie[];
     uint8 unrecognizedParameters[];
     cPacketPtr stateCookie;
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
    IPvXAddress initialDestination;
    uint32 msgNum;
    uint16 sid;
    bool ordered;
    uint32 booksize;
    uint32 rtx;
    bool fragment;
}

message SCTPDataChunk extends SCTPChunk
{
    // Chunk Flags
    bool eBit = 0;
    bool bBit = 0;
    bool uBit = 0;
    // Transmission Sequence Number
    uint32 tsn;
    // Stream identifier
    uint16 sid;
    // Stream Sequence Number
    uint16 ssn;
    // Payload Protocol Identifier
    uint32 ppid;
    simtime_t enqueuingTime;
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
    // Number of Duplicate TSNs
    uint16 numDupTsns;
    // Start and End of Gap Ack Blocks
    uint32 gapStart[];
    uint32 gapStop[];
    uint32 dupTsns[];
    uint32 sackSeqNum        = 0;
}

message SCTPAbortChunk extends SCTPChunk
{
     // Chunk Flags
     bool T_Bit = 0;
}

message SCTPHeartbeatChunk extends SCTPChunk
{
     IPvXAddress remoteAddr;
     simtime_t timeField;
     char info[];
}

message SCTPHeartbeatAckChunk extends SCTPChunk
{
     IPvXAddress remoteAddr;
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


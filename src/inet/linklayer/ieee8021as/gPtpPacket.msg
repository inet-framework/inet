//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
// 

import inet.common.packet.chunk.Chunk;

namespace inet;

enum gPtpMessageType
{
    GPTPTYPE_SYNC    = 1010;
    GPTPTYPE_FOLLOW_UP = 1011;
    GPTPTYPE_PDELAY_REQ = 1014;
    GPTPTYPE_PDELAY_RESP = 1012;
    GPTPTYPE_PDELAY_RESP_FOLLOW_UP = 1013;
}

enum gPtpPacketSize
{
    GPTP_SYNC_PACKET_SIZE = 44;
    GPTP_FOLLOW_UP_PACKET_SIZE = 76;
    GPTP_PDELAY_REQ_PACKET_SIZE = 54;
    GPTP_PDELAY_RESP_PACKET_SIZE = 54;
    GPTP_PDELAY_RESP_FOLLOW_UP_PACKET_SIZE = 54;
}

class GPtpBase extends FieldsChunk
{
    int typeCode;
}

class GPtpSync extends GPtpBase
{
    typeCode = GPTPTYPE_SYNC;
    chunkLength = B(GPTP_SYNC_PACKET_SIZE);
    simtime_t sentTime;
    simtime_t localDrift;
    simtime_t originTimestamp;  
}

class GPtpFollowUp extends GPtpBase
{
    typeCode = GPTPTYPE_FOLLOW_UP;
    chunkLength = B(GPTP_FOLLOW_UP_PACKET_SIZE);
    simtime_t sentTime;
    simtime_t preciseOriginTimestamp;
    simtime_t correctionField;
    simtime_t rateRatio;
    int numberOfHops;
}

class GPtpPdelayReq extends GPtpBase
{
    typeCode = GPTPTYPE_PDELAY_REQ;
    chunkLength = B(GPTP_PDELAY_REQ_PACKET_SIZE);
    simtime_t sentTime;
    simtime_t originTimestamp;
    int requestingPortIdentity;
}

class GPtpPdelayResp extends GPtpBase
{
    typeCode = GPTPTYPE_PDELAY_RESP;
    chunkLength = B(GPTP_PDELAY_RESP_PACKET_SIZE);
    simtime_t sentTime;
    simtime_t requestReceiptTimestamp;
    int requestingPortIdentity;
}

class GPtpPdelayRespFollowUp extends GPtpBase
{
    typeCode = GPTPTYPE_PDELAY_RESP_FOLLOW_UP;
    chunkLength = B(GPTP_PDELAY_RESP_FOLLOW_UP_PACKET_SIZE);
    simtime_t sentTime;
    simtime_t responseOriginTimestamp;
    int requestingPortIdentity;
}


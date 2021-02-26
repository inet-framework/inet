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

import inet.clock.common.ClockEvent;
import inet.clock.common.ClockTime;
import inet.common.TagBase;
import inet.common.packet.chunk.Chunk;
import inet.common.packet.Packet;

cplusplus {{
#include "inet/common/Units.h"
#include "inet/clock/common/ClockEvent.h"

namespace inet {
using namespace inet::units::values;
const B GPTP_SYNC_1STEP_PACKET_SIZE = B(76);
const B GPTP_SYNC_2STEP_PACKET_SIZE = B(44);
const B GPTP_FOLLOW_UP_PACKET_SIZE = B(76);
const B GPTP_PDELAY_REQ_PACKET_SIZE = B(54);
const B GPTP_PDELAY_RESP_PACKET_SIZE = B(54);
const B GPTP_PDELAY_RESP_FOLLOW_UP_PACKET_SIZE = B(54);
}

}}

namespace inet;

enum GptpNodeType
{
    MASTER_NODE = 11;
    BRIDGE_NODE = 12;
    SLAVE_NODE  = 13;
}

enum GptpPortType
{
    MASTER_PORT  = 2;
    SLAVE_PORT   = 1;
    PASSIVE_PORT = 0;
}

enum GptpMessageType
{
    GPTPTYPE_SYNC    = 0x0;
    GPTPTYPE_FOLLOW_UP = 0x8;
    GPTPTYPE_PDELAY_REQ = 0x2;
    GPTPTYPE_PDELAY_RESP = 0x3;
    GPTPTYPE_PDELAY_RESP_FOLLOW_UP = 0xA;
}

enum GptpSelfMsgKind {
    GPTP_SELF_REQ_ANSWER_KIND = 101;
    GPTP_SELF_MSG_SYNC = 103;
    GPTP_REQUEST_TO_SEND_SYNC = 104;
    GPTP_SELF_MSG_PDELAY_REQ = 105;
}

// ieee802.1AS-2020 10.6.2.2.8: flags (Octet2)
enum GptpFlags {
    twoStepFlag = 2;
}

//struct Timestamp
//{
//    UInteger48 seconds;
//    UInteger32 nanoseconds;
//};

//struct ExtendedTimestamp
//{
//    UInteger48 seconds;
//    UInteger48 fractionalNanoseconds;
//};

//typedef Octet8 ClockIdentity;

struct PortIdentity
{
    uint64_t clockIdentity;
    uint16_t portNumber;
};

//struct ClockQuality
//{
//    UInteger8 clockClass;
//    Enumeration8 clockAccuracy;
//    UInteger16 offsetScaledLogVariance;
//};

message GptpReqAnswerEvent extends ClockEvent
{
    int portId;
    clocktime_t ingressTimestamp;
    PortIdentity sourcePortIdentity;
    uint16_t sequenceId;
}

// ieee802.1AS-2020 11.4.2
class GptpBase extends FieldsChunk
{
    GptpMessageType messageType;
    uint16_t messageLength; // currently unused
    uint8_t domainNumber; // currently unused
    uint16_t flags; // currently unused
    clocktime_t correctionField;
    PortIdentity sourcePortIdentity;
    uint16_t sequenceId; // currently unused
    uint8_t logMessageInterval; // currently unused
}

class GptpSync extends GptpBase
{
    messageType = GPTPTYPE_SYNC;
    flags = twoStepFlag;
    chunkLength = GPTP_SYNC_2STEP_PACKET_SIZE;
    clocktime_t originTimestamp; // filled when twoStep flag is FALSE
    // followUpInformationTLV; // filled when twoStep flag is FALSE
}

class GptpFollowUp extends GptpBase
{
    messageType = GPTPTYPE_FOLLOW_UP;
    chunkLength = GPTP_FOLLOW_UP_PACKET_SIZE;
    clocktime_t preciseOriginTimestamp;  // 11.4.4.2.1
    // followUpInformationTLV:
    double rateRatio;  // 11.4.4.3.6 cumulativeScaledRateOffset, relative to GrandMaster
    // uint16_t gmTimeBaseIndicator;
    // ScaledNs lastGmPhaseChange;
    // int32_t scaledLastGmFreqChange;
    // ??? int numberOfHops;
}

class GptpPdelayReq extends GptpBase
{
    messageType = GPTPTYPE_PDELAY_REQ;
    chunkLength = GPTP_PDELAY_REQ_PACKET_SIZE;
    clocktime_t reserved1;
    clocktime_t reserved2;
}

class GptpPdelayResp extends GptpBase
{
    messageType = GPTPTYPE_PDELAY_RESP;
    chunkLength = GPTP_PDELAY_RESP_PACKET_SIZE;
    clocktime_t requestReceiptTimestamp;
    PortIdentity requestingPortIdentity;
}

class GptpPdelayRespFollowUp extends GptpBase
{
    messageType = GPTPTYPE_PDELAY_RESP_FOLLOW_UP;
    chunkLength = GPTP_PDELAY_RESP_FOLLOW_UP_PACKET_SIZE;
    clocktime_t responseOriginTimestamp;
    PortIdentity requestingPortIdentity;
}

class GptpIngressTimeInd extends TagBase
{
    clocktime_t arrivalClockTime;
}


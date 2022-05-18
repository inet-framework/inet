//
// Copyright (C) 2011 CoCo Communications
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.contract.ipv4.Ipv4Address;
import inet.transportlayer.common.CrcMode;

namespace inet;

enum IgmpTimerKind
{
    IGMP_QUERY_TIMER = 1001;
    IGMP_HOSTGROUP_TIMER = 1002;
    IGMP_LEAVE_TIMER = 1003;
    IGMP_REXMT_TIMER = 1004;
};

enum IgmpType
{
    IGMP_MEMBERSHIP_QUERY = 0x11;
    IGMPV1_MEMBERSHIP_REPORT = 0x12;
    IGMPV2_MEMBERSHIP_REPORT = 0x16;
    IGMPV2_LEAVE_GROUP = 0x17;
    IGMPV3_MEMBERSHIP_REPORT = 0x22;
}

class IgmpMessage extends FieldsChunk
{
    IgmpType type;         // 1 byte
    uint16_t crc = 0;    // 2 bytes, The checksum is the 16-bit one's complement of the one's complement sum of the whole IGMP message (the entire IP payload).
    CrcMode crcMode = CRC_MODE_UNDEFINED;  // 0 bytes, inet only
}

class IgmpQuery extends IgmpMessage
{
    type = IGMP_MEMBERSHIP_QUERY;
    Ipv4Address groupAddress;
}

class Igmpv1Query extends IgmpQuery
{
    chunkLength = B(8);
    uint8_t unused = 0;
}

class Igmpv1Report extends IgmpMessage
{
    chunkLength = B(8);
    type = IGMPV1_MEMBERSHIP_REPORT;
    uint8_t unused = 0;
    Ipv4Address groupAddress;
}

class Igmpv2Query extends IgmpQuery      //FIXME Igmpv1Query and Igmpv2Query differs only value of code field.
{
    chunkLength = B(8);
    uint8_t maxRespTimeCode;      // stored in `code` in units of 1/10 second, 0.1-25.5s
}

class Igmpv2Report extends IgmpMessage
{
    chunkLength = B(8);
    type = IGMPV2_MEMBERSHIP_REPORT;
    uint8_t maxRespTime = 0;
    Ipv4Address groupAddress;
}

class Igmpv2Leave extends IgmpMessage
{
    chunkLength = B(8);
    type = IGMPV2_LEAVE_GROUP;
    uint8_t maxRespTime = 0;
    Ipv4Address groupAddress;
}

//
// Igmpv3Query extends Igmpv2Query, because
// Igmpv2 routers must accept Igmpv3Query packets.
//
// Note: in Igmpv3Query the maxResponseTime field is
// renamed to maxResponseCode and it is interpreted
// differently than in Igmpv2.
class Igmpv3Query extends Igmpv2Query
{
    chunkLength = B(12);    // 12 + sourceListArraySize * 4
    // maxRespTime stored in `code` in units of 1/10 second
    //     0-127: 0-12.7s
    //   128-255: 1eeemmmm : (mant | 0x10) << (exp + 3)
    uint8_t resv = 0;    // 4 bit
    bool suppressRouterProc;    // 1 bit, S flag
    uint8_t robustnessVariable;    // 3 bit, QRV
    uint8_t queryIntervalCode;    // 8 bit, QQIC, in seconds, format similar to macRespTime (0nnnnnnn or 1eeemmmm)
    Ipv4AddressVector sourceList;    // 16 + n*32 bit
}

enum GroupRecordType
{
    MODE_IS_INCLUDE = 1;
    MODE_IS_EXCLUDE = 2;
    CHANGE_TO_INCLUDE_MODE = 3;
    CHANGE_TO_EXCLUDE_MODE = 4;
    ALLOW_NEW_SOURCES = 5;
    BLOCK_OLD_SOURCE = 6;
}

class GroupRecord
{
    @packetData;
    int recordType enum(GroupRecordType);
    Ipv4Address groupAddress;
    Ipv4AddressVector sourceList;
    uint32_t auxData[];
};

class Igmpv3Report extends IgmpMessage
{
    type = IGMPV3_MEMBERSHIP_REPORT;
    uint8_t resv1 = 0;
    uint16_t resv2 = 0;
    GroupRecord groupRecord[];
}

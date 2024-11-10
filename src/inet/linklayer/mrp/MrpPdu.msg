//
// Copyright (C) 2024 Daniel Zeitler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.linklayer.common.MacAddress;

namespace inet;

// Note: The data structures defined below are based on "Table 22 – MRP Substitutions"
// on pp.62 of the IEC-62439-2 specification.

//TODO there seem to be MRP_TestPropagate and MRP_TestMgrNAck frames too, wich are not covered here

//
// "Type" values for TLVs in the MRP protocol.
//
enum TlvHeaderType {
    END = 0x00;
    COMMON = 0x01;
    TEST = 0x02;
    TOPOLOGYCHANGE = 0x03;
    LINKDOWN = 0x04;
    LINKUP = 0x05;
    INTEST = 0x06;
    INTOPOLOGYCHANGE = 0x07;
    INLINKDOWN = 0x08;
    INLINKUP = 0x09;
    INLINKSTATUSPOLL = 0x0A;
    OPTION = 0x7F;
};

enum SubTlvHeaderType
{
    RESERVED = 0x00;
    TEST_MGR_NACK = 0x01;
    TEST_PROPAGATE = 0x02;
    AUTOMGR = 0x03;
};

//
// MRP_Version (non-TLV field): Specifies the version of the MRP being used.
//
class MrpVersion extends FieldsChunk
{
    chunkLength = B(2);
    uint16_t version = 1;
}

//
// MRP_TLVHeader: Base class for all TLVs in MRP; contains the Type and
// Length fields of the TLV. Note that all TLVs (except for MRP_End)
// must be padded to a multiple of 32 bits.
//
class MrpTlvHeader extends FieldsChunk
{
    TlvHeaderType headerType;
    uint8_t valueLength;
}

//
// MRP_End: Indicates the end of a list of TLVs in an MRP message.
//
class MrpEnd extends MrpTlvHeader
{
    chunkLength = B(2);
    headerType = END;
    valueLength = 0;
}

//
// MRP_Test: Periodically sent by the Media Redundancy Manager (MRM) on both of
// its ring ports to check the ring's operational status. Test frames circulate
// through all devices in the ring, and must be received by the MRM on its
// opposite ring port to confirm ring integrity.
//
class MrpTest extends MrpTlvHeader
{
    chunkLength = B(20);
    headerType = TEST;
    valueLength = 18;
    uint16_t prio = 0x8000;  // MRP_Prio, frame priority
    MacAddress sa;   // MRP_SA, source MAC address
    uint16_t portRole;
    uint16_t ringState;
    uint16_t transition;
    uint32_t timeStamp; // MRP_TimeStamp, in milliseconds
}

//
// MRP_TopologyChange: Sent by the Media Redundancy Manager (MRM) to Media
// Redundancy Clients (MRCs) to indicate a change in network topology, typically
// due to a link change. Recipients should react by clearing their MAC address
// tables (FDB).
//
class MrpTopologyChange extends MrpTlvHeader
{
    chunkLength = B(12);
    headerType = TOPOLOGYCHANGE;
    valueLength = 10;
    uint16_t prio;
    MacAddress sa;
    uint16_t interval; // in milliseconds
}

//
// MRP_LinkChange: MRP_LinkUp or MRP_LinkDown (depends on headerType).
// Sent by Media Redundancy Clients (MRCs) to report changes in
// link status to the MRM.
//
class MrpLinkChange extends MrpTlvHeader
{
    chunkLength = B(12);
    headerType = LINKDOWN; // LINKUP or LINKDOWN
    valueLength = 10;
    MacAddress sa;
    uint16_t interval; // in milliseconds
    uint16_t blocked;
}

//
// MRP_InTest: Periodically sent by the Media Redundancy Interconnection Manager
// (MIM) on both of its interconnection ports to check the integrity of the
// interconnection between the two MRP rings. These frames are circulated among
// all interconnected MIMs and MICs, and must be received back at the
// originating MIM to confirm the integrity of the interconnection.
//
class MrpInTest extends MrpTlvHeader
{
    chunkLength = B(20);
    headerType = INTEST;
    valueLength = 18;
    uint16_t inID = 0x8000;
    MacAddress sa;
    uint16_t portRole;
    uint16_t inState;
    uint16_t transition;
    uint32_t timeStamp;  // in milliseconds
}

//
// MRP_InLinkChange: MRP_InLinkDown or MRP_InLinkUp (depends on headerType).
// Sent by the Media Redundancy Interconnection Clients (MICs) to the Media
// Redundancy Interconnection Manager (MIM) to inform the MIM of any changes in
// the link status within the interconnection setup.
//
class MrpInLinkChange extends MrpTlvHeader
{
    chunkLength = B(16);
    headerType = INLINKDOWN; // INLINKDOWN or INLINKUP
    valueLength = 14;
    MacAddress sa;
    uint16_t portRole;
    uint16_t inID;
    uint16_t interval; // in milliseconds
    uint16_t linkInfo; // additional link info such as reason for failure
}

//
// MRP_InLinkStatusPoll: Sent by the Media Redundancy Interconnection Manager
// (MIM) to the Media Redundancy Interconnection Clients (MICs) within the
// interconnected MRP rings. The MICs respond with MRP_InLinkChange frames,
// providing the current status of their interconnection links to inform the MIM
// of any changes or confirm their operational status.
//
class MrpInLinkStatusPoll extends MrpTlvHeader
{
    chunkLength = B(12);
    headerType = INLINKSTATUSPOLL;
    valueLength = 10;
    MacAddress sa;
    uint16_t portRole;
    uint16_t inID;
}

//
// MRP_InTopologyChange: Sent by the MIM to all connected Media Redundancy
// Interconnection Clients (MICs) and the Media Redundancy Managers (MRMs) in
// the interconnected MRP rings to notify them of changes in the network
// topology, facilitating updates in network configuration and operation.
//
class MrpInTopologyChange extends MrpTlvHeader
{
    chunkLength = B(12);
    headerType = INTOPOLOGYCHANGE;
    valueLength = 10;
    MacAddress sa;
    uint16_t inID;
    uint16_t interval; // in milliseconds
}

//
// MRP_Common: Carries common information relevant across various MRP messages.
//
class MrpCommon extends MrpTlvHeader
{
    chunkLength = B(20);
    headerType = COMMON;
    valueLength = 18;
    uint16_t sequenceID; // MRP_SequenceID
    uint64_t uuid0;  // MRP_DomainUUID (128-bit field)
    uint64_t uuid1;
}

//
// MRP_SubTLVHeader: Generic header for SubTLVs used in more detailed MRP configurations.
//
class MrpSubTlvHeader extends FieldsChunk // equals to suboption2 and Mrp_AutoMgr
{
    chunkLength = B(2);
    SubTlvHeaderType subType = AUTOMGR;
    uint8_t subHeaderLength = 0;
}

//
// MRP_ManufacturerFkt: Placeholder for manufacturer-specific data or functionalities.
//
class MrpManufacturerFkt extends MrpSubTlvHeader
{
    chunkLength = B(2);
    subType = RESERVED;
    subHeaderLength= 0;
    // Followed By DataChunk ManufacturerData, set valueLength accordingly
    // BytesChunk manufacturerData;
}

//
// MRP_SubTLVTest: Used for testing purposes within subTLV structures,
// not typically part of standard MRP.
//
class MrpSubTlvTest extends MrpSubTlvHeader
{
    chunkLength = B(18);
    subType = TEST_PROPAGATE; // alternative: TEST_MGR_NACK
    subHeaderLength = 16;
    uint16_t prio;
    MacAddress sa;
    uint16_t otherMRMPrio;
    MacAddress otherMRMSa;
}

//
// Values for the vendor part of the MAC address,
// also used in MrpOption.
//
enum MrpOuiType {
    OUI = 0x000000;
    IEC = 0x00154E;
};

//
// Contents of Table 25 - Lengths of MRP_Ed1ManufacturerData for different
// MRP_Ed1Type values in MRP_Option.
//
enum Ed1DataLength {
    LENGTH0 = 2;    // for Ed1 type 0x00
    LENGTH13 = 0;   // for Ed1 type 0x01-0x03
    LENGTH4 = 26;   // for Ed1 type 0x04
    LENGTH5FE = 0;  // for Ed1 type 0x05-0xFE
    LENGTHFF = 0;   // for Ed1 type 0xFF
};

//
// MRP_Option: Contains optional, vendor-specific extensions to the MRP messages.
//
class MrpOption extends MrpTlvHeader
{
    chunkLength = B(8);
    headerType = OPTION;
    valueLength = 4;
    MrpOuiType ouiType = IEC;
    //suboption1
    uint8_t ed1Type = 0xFF;
    //followed by ManufacturerData of length 2 for ed1Type 0x00 and length 26 for ed1Type 0x04
    //BytesChunk manufacturerData;
    //subTlvHeader subOption2;
}

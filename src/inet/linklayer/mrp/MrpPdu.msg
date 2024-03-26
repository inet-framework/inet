// Copyright (C) 2024 Daniel Zeitler
// SPDX-License-Identifier: LGPL-3.0-or-later

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.linklayer.common.MacAddress;

namespace inet;

//vendor Part of mac-Adress
enum MrpOuiType {
    OUI = 0x000000;
    IEC = 0x00154E;
};


enum Ed1DataLength {
    LENGTH0 = 2; 	//tpye ed1 0x00
    LENGTH13 = 0; //type ed1 0x01-0x03
    LENGTH4 = 26; //type ed1 0x04
    LENGTH5FE = 0; //type ed1 0x05-0xFE
    LENGTHFF = 0; //type ed1 0xFF
};
    

enum TlvHeaderType {
    END = 0x00;
    COMMON = 0x01;
    TEST = 0x02;
    TOPOLOGYCHANGE=0x03;
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
	TEST_MGR_NACK= 0x01;
	TEST_PROPAGATE = 0x02;
	AUTOMGR = 0x03;
};

//
// MRP_TLVHeader
//
class MrpTlvHeader extends FieldsChunk
{
    TlvHeaderType headerType;
    uint8_t headerLength;
}

//
// MRP_End
//
class MrpEnd extends MrpTlvHeader
{
    chunkLength = B(2);
    headerType = END;
    headerLength = 0;
}

//
// MRP_Test
//
class MrpTest extends MrpTlvHeader
{
    chunkLength = B(20);
    headerType = TEST;
    headerLength = 18;
    uint16_t prio= 0x8000;
    MacAddress sa;
    uint16_t portRole;
    uint16_t ringState;
    uint16_t transition;
    uint32_t timeStamp; // in milliseconds
}

//
// MRP_TopologyChange
//
class MrpTopologyChange extends MrpTlvHeader
{
    chunkLength = B(16);
    headerType = TOPOLOGYCHANGE;
    headerLength = 14;
    uint16_t prio;
    MacAddress sa;
    uint16_t portRole;
    uint16_t interval; // in milliseconds
}

//
// MRP_LinkChange
//
class MrpLinkChange extends MrpTlvHeader
{
    chunkLength = B(16);
    headerType = LINKDOWN;
    headerLength = 14;
    MacAddress sa;  
    uint16_t portRole;
    uint16_t interval; // in milliseconds 
    uint16_t blocked;
}

//
// MRP_InTest
//
class MrpInTest extends MrpTlvHeader
{
    chunkLength = B(20);
    headerType = INTEST;
    headerLength = 18;
    uint16_t inID = 0x8000;
    MacAddress sa;
    uint16_t portRole;
    uint16_t inState;
    uint16_t transition;
    uint32_t timeStamp;  // in milliseconds
}

//
// MRP_InLinkChange
//
class MrpInLinkChange extends MrpTlvHeader
{
    chunkLength = B(16);
    headerType = INLINKDOWN;
    headerLength = 14;
    MacAddress sa;  
    uint16_t portRole;
    uint16_t inID;
    uint16_t interval; // in milliseconds 
    uint16_t linkInfo; // additional link info such as reason for failure
}

//
// MRP_InLinkStatusPoll
//
class MrpInLinkStatusPoll extends MrpTlvHeader
{
	chunkLength = B(12);
	headerType = INLINKSTATUSPOLL;
	headerLength = 10;
	MacAddress sa;
	uint16_t portRole;
	uint16_t inID;
}

//
// MRP_InTopologyChange
//
class MrpInTopologyChange extends MrpTlvHeader
{
    chunkLength = B(12);
    headerType = INTOPOLOGYCHANGE;
    headerLength = 10;
    MacAddress sa;
    uint16_t inID;
    uint16_t interval; // in milliseconds
}

//
// MRP_Common
//
class MrpCommon extends MrpTlvHeader
{
    chunkLength = B(20);
    headerType = COMMON;
    headerLength = 18;
    uint16_t sequenceID;
    uint64_t uuid0;
    uint64_t uuid1;
}

//
// MRP_SubTLVHeader
//
class MrpSubTlvHeader extends FieldsChunk //equals to suboption2 and Mrp_AutoMgr
{
    chunkLength = B(2);
    SubTlvHeaderType subType = AUTOMGR;  //default set to automanager
    uint8_t subHeaderLength = 0;	
}

//
// MRP_ManufacturerFkt
//
class MrpManufacturerFkt extends MrpSubTlvHeader
{
    chunkLength = B(2);
    subType = RESERVED;
    subHeaderLength= 0;
    //Followed By DataChunk ManufacturerData, set headerLength accordingly
    //BytesChunk manufacturerData;
}

//
// MRP_SubTLVTest
//
class MrpSubTlvTest extends MrpSubTlvHeader
{
    chunkLength = B(18);
    subType = TEST_PROPAGATE; //alternativ: TEST_MGR_NACK
    subHeaderLength = 16;
    uint16_t prio;
    MacAddress sa;
    uint16_t otherMRMPrio;
    MacAddress otherMRMSa;
}

//
// MRP_Option
//
class MrpOption extends MrpTlvHeader
{
    chunkLength = B(6);
    headerType = OPTION;
    headerLength = 4;
    MrpOuiType ouiType = IEC;
    //suboption1
    uint8_t ed1Type = 0xFF;
    //followed by ManufacturerData of length 2 for ed1Type 0x00 and length 26 for ed1Type 0x04
	//BytesChunk manufacturerData;
	//subTlvHeader subOption2;
}

//
// MRP_Version (non-TLV field) 
//
class MrpVersion extends FieldsChunk
{
    chunkLength = B(2);
    uint16_t version = 1;
}

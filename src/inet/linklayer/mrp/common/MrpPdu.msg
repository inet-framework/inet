// Copyright (C) 2024 Daniel Zeitler
// SPDX-License-Identifier: LGPL-3.0-or-later

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.linklayer.common.MacAddress;

namespace inet;

//vendor Part of mac-Adress
enum mrpOuiType {
    OUI = 0x000000;
    IEC = 0x00154E;
};


enum ed1DataLength {
    LENGTH0 = 2; 	//tpye ed1 0x00
    LENGTH13 = 0; //type ed1 0x01-0x03
    LENGTH4 = 26; //type ed1 0x04
    LENGTH5FE = 0; //type ed1 0x05-0xFE
    LENGTHFF = 0; //type ed1 0xFF
};
    

enum tlvHeaderType {
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

enum subTlvHeaderType
{
    RESERVED = 0x00;
	TEST_MGR_NACK= 0x01;
	TEST_PROPAGATE = 0x02;
	AUTOMGR = 0x03;
};

class tlvHeader extends FieldsChunk //equals endHeader
{
    chunkLength = B(2);
    tlvHeaderType headerType = END; 
    uint8_t headerLength = 0;
	                          
}

class testFrame extends tlvHeader
{
    chunkLength = B(20);
    headerType = TEST;
    headerLength = 18;
    uint16_t prio= 0x8000;
    MacAddress sa;
    uint16_t portRole;
    uint16_t RingState;
    uint16_t Transition;
    uint32_t timeStamp;
}

class topologyChangeFrame extends tlvHeader
{
    chunkLength = B(16);
    headerType = TOPOLOGYCHANGE;
    headerLength = 14;
    uint16_t prio;
    MacAddress sa;
    uint16_t portRole;
    uint16_t interval;
}

class linkChangeFrame extends tlvHeader
{
    chunkLength = B(16);
    headerType = LINKDOWN;
    headerLength = 14;
    MacAddress sa;  
    uint16_t portRole;
    uint16_t interval; 
    uint16_t blocked;
}

class inTestFrame extends tlvHeader
{
    chunkLength = B(20);
    headerType = INTEST;
    headerLength = 18;
    uint16_t inID = 0x8000;
    MacAddress sa;
    uint16_t portRole;
    uint16_t inState;
    uint16_t Transition;
    uint32_t timeStamp;
}

class inLinkChangeFrame extends tlvHeader
{
    chunkLength = B(16);
    headerType = INLINKDOWN;
    headerLength = 14;
    MacAddress sa;  
    uint16_t portRole;
    uint16_t inID;
    uint16_t interval; 
    uint16_t LinkInfo;
}

class inLinkStatusPollFrame extends tlvHeader
{
	chunkLength = B(12);
	headerType = INLINKSTATUSPOLL;
	headerLength = 10;
	MacAddress sa;
	uint16_t portRole;
	uint16_t inID;
}

class inTopologyChangeFrame extends tlvHeader
{
    chunkLength = B(12);
    headerType = INTOPOLOGYCHANGE;
    headerLength = 10;
    MacAddress sa;
    uint16_t inID;
    uint16_t interval;
}

class commonHeader extends tlvHeader
{
    chunkLength = B(20);
    headerType = COMMON;
    headerLength = 18;
    uint16_t SequenceID;
    uint64_t uuid0;
    uint64_t uuid1;
}

class subTlvHeader extends FieldsChunk //equals to suboption2 and Mrp_AutoMgr
{
    chunkLength = B(2);
    subTlvHeaderType subType = AUTOMGR;  //default set to automanager
    uint8_t subHeaderLength = 0;	
}

class manufacturerFktHeader extends subTlvHeader
{
    chunkLength = B(2);
    subType = RESERVED;
    subHeaderLength= 0;
    //Followed By DataChunk ManufacturerData, set headerLength accordingly
    //BytesChunk manufacturerData;
}

class subTlvTestFrame extends subTlvHeader
{
    chunkLength = B(18);
    subType = TEST_PROPAGATE; //alternativ: TEST_MGR_NACK
    subHeaderLength = 16;
    uint16_t prio;
    MacAddress sa;
    uint16_t otherMRMPrio;
    MacAddress otherMRMSa;
}

class optionHeader extends tlvHeader
{
    chunkLength = B(6);
    headerType = OPTION;
    headerLength = 4;
    mrpOuiType ouiType = IEC;
    //suboption1
    uint8_t ed1Type = 0xFF;
    //followed by ManufacturerData of length 2 for ed1Type 0x00 and length 26 for ed1Type 0x04
	//BytesChunk manufacturerData;
	//subTlvHeader subOption2;
}


class mrpVersionField extends FieldsChunk
{
    chunkLength = B(2);
    uint16_t version = 1;
}
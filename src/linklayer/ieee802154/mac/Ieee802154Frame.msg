cplusplus {{
#include "Ieee802154Const.h"
#include "Ieee802154Def.h"
#include "Ieee802154Enum.h"
#include "Ieee802154Field.h"
}}


struct FrameCtrl;

struct SuperframeSpec;

struct DevCapability;

struct GTSFields;

struct PendingAddrFields;

enum Ieee802154MacCmdType;

// The common part of 802.15.4 frames
// MHR (header) and MFR (FCS)


packet Ieee802154Frame
{
    //	MHR
    //	Frame control fieled (FCF)(16 bits) - Fig 35
    FrameCtrl frmCtrl;

    //	Sequence number field (8 bits)
    unsigned char bdsn;

    //	Addressing fields
    uint64_t dstPanId;
    uint64_t dstAddr;
    uint64_t srcPanId;
    uint64_t srcAddr;

    bool isIndirect = false;
    bool isGTS = false;

		//	MFR
		//	FCS is not modeled and only included in the length (2 bytes)
}


// *****************************************
// Format of 802.15.4 beacon frame - Fig 37
// *****************************************

packet Ieee802154BeaconFrame extends Ieee802154Frame
{
    // MAC payload in beacon frames
    SuperframeSpec sfSpec;

		//GTSDescriptor gtsList[7];
		//unsigned short gtsCount;		

		//PendingAddrFields paFields;
			//	Pending address fields (PAF) - Fig 39
				// Pending address specification (8 bits) - Fig 44
					//unsigned char numShortAddr;		// number of short addresses pending
					//unsigned char numExtAddr;		// number of extended addresses pending
				// Address list
					//unsigned short addrList[7];		// pending address list, including short and extended, max 7
}


// ********************************
// Format of 802.15.4 Data frame 	
// ********************************

packet Ieee802154DataFrame extends Ieee802154Frame
{
}

// ******************************		
// Format of 802.15.4 Ack frame
// ******************************

message Ieee802154AckFrame extends Ieee802154Frame
{
}

// ********************************				
// Format of 802.15.4 command frame
// ********************************

message Ieee802154CmdFrame extends Ieee802154Frame
{
    int cmdType enum(Ieee802154MacCmdType);

}

// *** Association request command - Fig 48 ***
message Ieee802154AssoReqtCmd extends Ieee802154CmdFrame
{
    DevCapability capability;
}

// *** Association response command (ARC) - Fig 50 ***
message Ieee802154AssoRespCmd extends Ieee802154CmdFrame
{
    // Command payload
    unsigned short shortAddr;		// short address field
																// 16 bits short address allocated by coordinator
																// 0xffff: association failed

    bool atCapacity;						// association status field - Table 68 (modified)
																// failed reason, true: at capacity, false: access denied

}

// *** Disassociation notification command (DNC)- Fig 51 ***
message Ieee802154DisassoNotiCmd extends Ieee802154CmdFrame
{
    // Command payload
    bool isCoor;		// disassociation reason - Table 69, true: coordinator, false: device
}

// *** Data request command - Fig 52 *** 
message Ieee802154DataReqtCmd extends Ieee802154CmdFrame
{
}

// *** PAN ID conflict notification command - Fig 53 *** 
message Ieee802154PanIdConflictNotiCmd extends Ieee802154CmdFrame
{
}

// *** Orphan notification command - Fig 54 *** 
message Ieee802154OrphanNotiCmd extends Ieee802154CmdFrame
{
}

// *** Beacon request command - Fig 55 *** 
message Ieee802154BeaconReqtCmd extends Ieee802154CmdFrame
{
}

// *** Coordinator realignment command (CRC) - Fig 56 *** 
message Ieee802154CoorRealignCmd extends Ieee802154CmdFrame
{
    // Command payload
    unsigned short panId;						// PAN identifier field
    unsigned short coorShortAddr;		// coordinator short address field
    unsigned char logiChannel;			// logical channel field
    unsigned short shortAddr;				// short address field, 16 bits

}

// *** GTS request command (GRC) - Fig 57 *** 
message Ieee802154GtsReqtCmd extends Ieee802154CmdFrame
{
    // Command payload
    unsigned char gtsLength;	// number of superframe slots for GTS, 4 bit
    bool recvOnly;		// true: receive-only GTS, false: transmit-only GTS
    bool isAllocation;		// ture: GTS allocation, false: GTS deallocation

}

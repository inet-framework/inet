///
/// @file   HybridPonFrame.msg
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   2009-06-30
/// 
/// @brief  Defines self-messages and frames for hybridpon models.
/// 
/// @remarks Copyright (C) 2009-2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


//cplusplus {{
//#include "Ethernet.h"
//#include "MACAddress.h"
//#include "Ieee802Ctrl_m.h" // for enums
//}}


class cPacketQueue;

//
// Self-message used for event handling in modules
//
packet HybridPonMessage
{
    int onuIdx;
};


//
// Common base class for classes representing PON frame types,
// containing their common header fields.
//
// Source and destination address are stored in data members.
// Other fields include:
//  - preamble: not stored (only contributes length)
//  - length: stored in cMessage length, not stored explicitly
//  - payload: stored a encapsulated packet (cMessage::encapsulate())
//  - crc: represented by cMessage::hasBitError()
//
packet HybridPonDsFrame extends cPacket
{
    // 8-Bit "Flags" field
    //======================================
    // Bit	Field			Values
    //======================================
    // 0-3	Frame Type		0 - Normal data
    //						1 - GRANT
    //						2 - REGISTER_REQ
    //						3 - REGISTER_ACK (?)
    //						4-15 - Unused
    short frameType;
    // 4	Force Report	0 - No action required
    //						1 - ONU should report in the corresponding upstream frame
    // 5-7	Unused	

    // The following are not actual fields of a frame; they rather provide
    // parameters of "service primitives" in service specifications related
    // with MAC and PHY layers.
    int channel;	// wavelength channel (serves as ONU ID like LLID in EPON) 
};

packet HybridPonDsDataFrame extends HybridPonDsFrame
{
	// Encapsulated frames
    cPacketQueue encapsulatedFrames;
};

packet HybridPonDsGrantFrame extends HybridPonDsFrame
{
    // 16-Bit "Grant"
    int grant;
};

packet HybridPonDsControlFrame extends HybridPonDsGrantFrame
{
    // Add fields
}; 

packet HybridPonUsFrame extends cPacket
{   
    // 16-Bit "Report"
    int report;
    
   	// Encapsulated frames
    cPacketQueue encapsulatedFrames;
    
    // The following are not actual fields of a frame; they rather provide
    // parameters of "service primitives" in service specifications related
    // with MAC and PHY layers.
 
    //    int lambda;			// wavelength channel
    int channel;	// wavelength channel (serves as ONU ID like LLID in EPON)    
};

packet HybridPonUsControlFrame extends cPacket
{
    // Add fields
};

packet DummyPacket extends HybridPonDsFrame
{
    int idx;
};

///
/// @file   HybridPonFrame.msg
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Tue Jun 30 12:19:39 2009
/// 
/// @brief  Defines 'HybridPonFrame' and 'HybridPonMessage' message objects.
/// 
/// @remarks Copyright (C) 2009 Kyeong Soo (Joseph) Kim. All rights reserved.
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


packet HybridPonMessage
{
    int onuIdx;
};


// packet IpPacket
// {
//     int srcAddress;
//     int dstnAddress;
//     long seqNumber;
// };


// packet EthFrame extends IpPacket
// {
//     int header;
//     int fcs;
// };


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
packet HybridPonFrame extends cPacket
{
    bool id;
    int lambda;
    int grant;
    int request;
    cPacketQueue encapsulatedFrames;
};


packet DummyPacket extends HybridPonFrame
{
    int idx;
};

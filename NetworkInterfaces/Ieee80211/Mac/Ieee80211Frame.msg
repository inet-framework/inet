//
// Copyright (C) 2006 Andras Varga
// Copyright (C) 2001 Eric Wu and Steve Woon, Monash University, Melbourne, Australia
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

//
// Required for MACAddress declarations
//
cplusplus
{{
#include "MACAddress.h"
}};
class noncobject MACAddress;

//
// 802.11 frame type constants, for the "type" field of Ieee80211FrameControl
//
enum Ieee80211FrameType
{
      FT_MANAGEMENT = 0;
      FT_CONTROL = 1;
      FT_DATA = 2;
      FT_RESERVED = 3;
};

//
// 802.11 frame subtype constants, for the "subtype" field of Ieee80211FrameControl.
// (Numeric values do not correspond to the standard, where actual frame type is
// determined by the type AND subtype fields together; here we chose unique
// values for convenience.)
//
enum Ieee80211SubType
{
      ST_ASSOCIATIONREQUEST = 4;
      ST_ASSOCIATIONRESPONSE = 5;
      ST_REASSOCIATIONREQUEST = 6;
      ST_REASSOCIATIONRESPONSE = 7;
      ST_PROBEREQUEST = 8;
      ST_PROBERESPONSE = 9;
      ST_BEACON = 10;
      ST_DISASSOCIATION = 11;
      ST_AUTHENTICATION = 12;
      ST_DEAUTHENTICATION = 13;
      ST_DATA = 14;
      ST_RTS = 15;
      ST_CTS = 16;
      ST_ACK = 17;
};

//
// Frame control format. Fields not supported by this model are omitted:
// ProtocolVersion, MoreFlag, PowerMgmt, MoreData, WEP, Order.
//
struct Ieee80211FrameControl
{
    fields:
        short type enum(Ieee80211FrameType); // 2 bits
        short subtype;                    // 4 bits
        bool toDS;                        // 1 bit
        bool fromDS;                      // 1 bit
        bool retry;                       // 1 bit
};

//
// Format of smallest 802.11 frame. FCS value is not
// explicitly modeled, but it is included in the length.
//
message Ieee80211Frame
{
    fields:
        byteLength = 14;
        Ieee80211FrameControl frameControl;
        short duration = -1; // "duration" in the Duration/ID field (-1=no duration)
        short AID = -1;      // "id" (Association ID) in the Duration/ID field (-1=no ID)
        MACAddress receiverAddress; // aka address1
};

//
// Format of a 802.11 frame with address1 present, like ACK and CTS
//
message Ieee80211OneAddressFrame extends Ieee80211Frame
{
};

//
// Format of the 802.11 ACK frame
//
message Ieee80211ACKFrame extends Ieee80211OneAddressFrame
{
};

//
// Format of a 802.11 frame with address1 and address2 present
//
message Ieee80211TwoAddressFrame extends Ieee80211OneAddressFrame
{
    fields:
        byteLength = 20;
        MACAddress transmitterAddress; // aka address2
};

//
// Format of the 802.11 RTS frame
//
message Ieee80211RTSFrame extends Ieee80211TwoAddressFrame
{
};

//
// Format of the 802.11 CTS frame
//
message Ieee80211CTSFrame extends Ieee80211OneAddressFrame
{
};

//
// Common base class for 802.11 data and management frames
//
message Ieee80211DataOrMgmtFrame extends Ieee80211TwoAddressFrame
{
    fields:
        byteLength = 28;
        MACAddress address3;
        short fragmentNumber;
        short sequenceNumber;
};

//
// Format of the 802.11 data frame
//
message Ieee80211DataFrame extends Ieee80211DataOrMgmtFrame
{
    fields:
        byteLength = 34;
        MACAddress address4;
};

//
// Base class for 802.11 management frames (subclasses will add frame body contents)
//
message Ieee80211ManagementFrame extends Ieee80211DataOrMgmtFrame
{
};


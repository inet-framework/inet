//
// Copyright (C) 2006 Andras Varga
// Copyright (C) 2001 Eric Wu and Steve Woon, Monash University, Melbourne, Australia
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.

//
// Required for MACAddress declarations
//
cplusplus {{
//FIXME use consts for frame lengths!!! account for LLC(3)+SNAP(5) headers in data frames!!! +add SNAP orgcode, localcode into the msg?
#include "MACAddress.h"
}}

class noncobject MACAddress;

//
// 802.11 frame type constants (type+subtype), for the "type" field of
// Ieee80211FrameControl
//
enum Ieee80211FrameType
{

    // management:
    ST_ASSOCIATIONREQUEST = 0x00;
    ST_ASSOCIATIONRESPONSE = 0x01;
    ST_REASSOCIATIONREQUEST = 0x02;
    ST_REASSOCIATIONRESPONSE = 0x03;
    ST_PROBEREQUEST = 0x04;
    ST_PROBERESPONSE = 0x05;
    ST_BEACON = 0x08;
    ST_ATIM = 0x09;
    ST_DISASSOCIATION = 0x0a;
    ST_AUTHENTICATION = 0x0b;
    ST_DEAUTHENTICATION = 0x0c;

    // control (CFEND/CFEND_CFACK omitted):
    ST_PSPOLL = 0x1a;
    ST_RTS = 0x1b;
    ST_CTS = 0x1c;
    ST_ACK = 0x1d;

    // data (CFPOLL/CFACK subtypes omitted):
    ST_DATA = 0x20;
}

//
// The common part of 802.11 frames.
//
// NOTE:
// FCS value is not explicitly modeled, but it is included in the length.
// Frame control format fields not supported by this model are omitted:
// MoreFlag, PowerMgmt, MoreData, WEP, Order.
//
packet Ieee80211Frame
{
    byteLength = 14;
    short type enum(Ieee80211FrameType); // type and subtype
    bool toDS;
    bool fromDS;
    bool retry;
    bool moreFragments;
    simtime_t duration = -1; // "duration" in the Duration/ID field (-1=no duration)
    short AID = -1;          // "id" (Association ID) in the Duration/ID field (-1=no ID)
    MACAddress receiverAddress; // aka address1
}

//
// Format of a 802.11 frame with address1 present, like ACK and CTS
//
packet Ieee80211OneAddressFrame extends Ieee80211Frame
{
}

//
// Format of the 802.11 ACK frame
//
packet Ieee80211ACKFrame extends Ieee80211OneAddressFrame
{
    type = ST_ACK;
}

//
// Format of a 802.11 frame with address1 and address2 present
//
packet Ieee80211TwoAddressFrame extends Ieee80211OneAddressFrame
{
    byteLength = 20;
    MACAddress transmitterAddress; // aka address2
}

//
// Format of the 802.11 RTS frame
//
packet Ieee80211RTSFrame extends Ieee80211TwoAddressFrame
{
    type = ST_RTS;
}

//
// Format of the 802.11 CTS frame
//
packet Ieee80211CTSFrame extends Ieee80211OneAddressFrame
{
    type = ST_CTS;
}

//
// Common base class for 802.11 data and management frames
//
packet Ieee80211DataOrMgmtFrame extends Ieee80211TwoAddressFrame
{
    byteLength = 28;
    MACAddress address3;
    short fragmentNumber;
    short sequenceNumber;
}

//
// Format of the 802.11 data frame
//
packet Ieee80211DataFrame extends Ieee80211DataOrMgmtFrame
{
    type = ST_DATA;
    byteLength = 34;
    MACAddress address4;
}

//
// Base class for 802.11 management frames (subclasses will add frame body contents)
//
packet Ieee80211ManagementFrame extends Ieee80211DataOrMgmtFrame
{
}


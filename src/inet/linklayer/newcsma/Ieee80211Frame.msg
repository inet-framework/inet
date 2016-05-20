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
#include "inet/linklayer/common/MACAddress.h"
}}

class noncobject MACAddress;

namespace inet::newcsma;

//
// 802.11 frame type constants (type+subtype), for the "type" field of
// Ieee80211FrameControl
//
enum Ieee80211FrameType
{
    ST_ACK = 0x1d;
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
    bool retry;
    MACAddress receiverAddress; // aka address1
}

//
// Format of a 802.11 frame with address1 present, like ACK
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
// Format of the 802.11 data frame
//
packet Ieee80211DataFrame extends Ieee80211TwoAddressFrame
{
    type = ST_DATA;
    byteLength = 34;
    MACAddress address4;
}

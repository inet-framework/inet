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
#include "inet/linklayer/common/MACAddress.h"
#include "inet/linklayer/common/Ieee802Ctrl_m.h" // for ~EtherType
}}

class noncobject MACAddress;

enum EtherType;

namespace inet::ieee80211;

cplusplus {{
const unsigned int LENGTH_RTS = 160;    //bits
const unsigned int LENGTH_CTS = 112;    //bits
const unsigned int LENGTH_ACK = 112;    //bits
const unsigned int LENGTH_MGMT = 28 * 8;    //bits
const unsigned int DATAFRAME_HEADER_MINLENGTH = 28 * 8;    //bits without QoS, without Address4, without SNAP: 2 + 2 + 3*6(addresses) + 2 + 4(crc)
const unsigned int QOSCONTROL_BITS = 2 * 8;    // QoS Control field length (bits)
const unsigned int SNAP_HEADER_BYTES = 8;
const short int MAX_NUM_FRAGMENTS = 16;
}}

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
    ST_ACTION = 0x0d;
    ST_NOACKACTION = 0x0e;

    // control (CFEND/CFEND_CFACK omitted):
    ST_PSPOLL = 0x1a;
    ST_RTS = 0x1b;
    ST_CTS = 0x1c;
    ST_ACK = 0x1d;
    ST_BLOCKACK_REQ = 0x18;
    ST_BLOCKACK     = 0x19;

    // data (CFPOLL/CFACK subtypes omitted):
    ST_DATA = 0x20;
    ST_DATA_WITH_QOS = 0x28;
    //Feedback frame for multicast tramsmission
    ST_LBMS_REQUEST = 0x30;
    ST_LBMS_REPORT = 0x31;
}

//
// Ack policy values for the 802.11 QoS control field
//
enum AckPolicy
{
    NORMAL_ACK = 0;
    NO_ACK = 1;
    NO_EXPLICIT_ACK = 2;
    BLOCK_ACK = 3;
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
    byteLength = LENGTH_ACK / 8;
    short type @enum(Ieee80211FrameType); // type and subtype
    bool toDS;
    bool fromDS;
    bool retry;
    bool moreFragments;
    simtime_t duration = -1; // "duration" in the Duration/ID field (-1=no duration)
    short AID = -1;          // "id" (Association ID) in the Duration/ID field (-1=no ID)
    MACAddress receiverAddress; // aka address1
    simtime_t MACArrive;    // FIXME remove it, technical data, used inside of MAC module
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
    byteLength = LENGTH_ACK / 8;
    type = ST_ACK;
}

//
// Format of a 802.11 frame with address1 and address2 present
//
packet Ieee80211TwoAddressFrame extends Ieee80211OneAddressFrame
{
    byteLength = LENGTH_RTS / 8;
    MACAddress transmitterAddress; // aka address2
}

//
// Format of the 802.11 RTS frame
//
packet Ieee80211RTSFrame extends Ieee80211TwoAddressFrame
{
    byteLength = LENGTH_RTS / 8;
    type = ST_RTS;
}

//
// Format of the 802.11 CTS frame
//
packet Ieee80211CTSFrame extends Ieee80211OneAddressFrame
{
    byteLength = LENGTH_CTS / 8;
    type = ST_CTS;
}

//
// Common base class for 802.11 data and management frames
//
packet Ieee80211DataOrMgmtFrame extends Ieee80211TwoAddressFrame
{
    byteLength = LENGTH_MGMT / 8;
    MACAddress address3;
    short fragmentNumber;
    uint16 sequenceNumber;
}

//
// Format of the 802.11 data frame
//
packet Ieee80211DataFrame extends Ieee80211DataOrMgmtFrame
{
//  The presence of the Address 4 field is determined by the setting of the To DS and From DS subfields of
//  the Frame Control field (see below). The QoS Control field is present when the QoS subfield of the Subtype
//  field is set to 1 (8.3.2.1 Data frame format)
//  FIXME: byteLength is not a constant value (DATAFRAME_HEADER_MINLENGTH / 8 = 28).
//  For example, with Address 4 and without QoS the correct length is 34 bytes.
//  Without Address 4 and QoS the correct value is indeed 28.
//  With Address 4 and QoS the correct value is indeed 36.
    byteLength = DATAFRAME_HEADER_MINLENGTH / 8;
    type = ST_DATA;         // or ST_DATA_WITH_QOS
    MACAddress address4;    // optional field, exists when toDS and fromDS are true
    uint16 qos;             // TODO nobody uses this currently, only its parts like tid below... 
    int ackPolicy @enum(AckPolicy); //FIXME in real life this is part of the optional qos field above...
    uint8 tid; //FIXME in real life this is part of the optional qos field above...
}

//TODO this can be introduced when the SNAP header 
//packet Ieee80211QoSDataFrame extends Ieee80211DataFrame
//{
//    type = ST_DATA_WITH_QOS;
//    byteLength = (DATAFRAME_HEADER_MINLENGTH + QOSCONTROL_BITS) / 8;
//    int ackPolicy @enum(AckPolicy); //FIXME in real life this is part of the optional qos field above...
//    uint8 tid;
//    // TODO: other QoS control field parts
//}

//
// 802.11 data frame with the 8-byte SNAP header (AA AA 03, 00 00 00, <2-byte ~EtherType>)
//
packet Ieee80211DataFrameWithSNAP extends Ieee80211DataFrame
{
    byteLength = DATAFRAME_HEADER_MINLENGTH / 8 + SNAP_HEADER_BYTES;
    int etherType @enum(EtherType);
}

//
// Base class for 802.11 management frames (subclasses will add frame body contents)
//
packet Ieee80211ManagementFrame extends Ieee80211DataOrMgmtFrame
{
    short Category;
    //short action @enum(WirelessNetworkManagementAction); // action
}

//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
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


cplusplus {{
#include "inet/linklayer/ethernet/Ethernet.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/linklayer/common/Ieee802Ctrl_m.h" // for enums
}}

namespace inet;

class noncobject MACAddress;

class noncobject MessageId;

enum EtherType;

enum SAPCode;

//
// This class should never be instantiated by the models, only specific
// subclasses: EthernetJam, EtherIFG and submodules of ~EtherFrame.
//
packet EtherTraffic
{
}

//
// Represents jam on the Ethernet.
//
packet EtherJam extends EtherTraffic
{
    long abortedPkTreeID = 0;  // this field stores the packetTreeId of the aborted packet
}

//
// Represents a filled inter-frame gap in burst mode.
// Also used for calculating IFG times.
//
packet EtherIFG extends EtherTraffic
{
    bitLength = INTERFRAME_GAP_BITS;
}

//
// Common base class for classes representing Ethernet II and 802.3 frame types,
// containing their common header fields.
//
// This class should never be instantiated by the models, only specific
// subclasses: ~EthernetIIFrame, ~EtherFrameWithLLC and ~EtherFrameWithSNAP.
//
// Source and destination MAC address are stored in data members.
// Other Ethernet header fields include:
//  - preamble: not stored (only contributes to length)
//  - length: stored in cPacket byteLength; during tranmission, cPacket byteLength
//    includes pyhsical layer overhead and the frameByteLength field stores the
//    actual frame length
//  - payload: stored a encapsulated packet (cPacket::encapsulate())
//  - crc: represented by cPacket::hasBitError()
//
packet EtherFrame extends EtherTraffic
{
    MACAddress dest;
    MACAddress src;
    int frameByteLength;  // frame length without physical layer overhead (preamble, SFD, carrier extension); used by MAC layer
}


//
// Ethernet II headers contain a 16-bit ~EtherType to identify
// the encapsulated protocol.
//
// Header length: src(6)+dest(6)+etherType(2) = 14 bytes
//
packet EthernetIIFrame extends EtherFrame
{
    int etherType @enum(EtherType);
}


//
// Ethernet frame with 802.3 LLC header.
//
// Header length: src(6)+dest(6)+length(2)+ssap(1)+dsap(1)+control(1) = 17 bytes
//
packet EtherFrameWithLLC extends EtherFrame
{
    int dsap;
    int ssap;
    int control;
}


//
// Ethernet frame with 802.3 LLC and SNAP headers.
//
// The ssap, dsap and control LLC fields are set to fixed values in the
// frame: 0xAA, 0xAA, 0x03.
//
// Header length: src(6)+dest(6)+length(2)+ssap(1)+dsap(1)+control(1)+
// orgCode(3)+localCode(2) = 22 bytes
//
packet EtherFrameWithSNAP extends EtherFrameWithLLC
{
    dsap = 0xAA;
    ssap = 0xAA;
    control = 0x03;
    int orgCode;   // organizationally unique identifier (OUI); 0 for protocols that have an EtherType (ARP, IPv4, IPv6, etc.)
    int localcode; // protocol identifier (PID); stores ~EtherType if orgCode is 0
}


//
// Ethernet frame used by the PAUSE protocol
//
packet EtherPauseFrame extends EtherFrame
{
    int pauseTime; // in 512 bit-time units
}

// Mod1
// Ethernet 802.1Q Tag. VLAN
//
//
packet Ethernet1QTag
{
    uint8_t pcp;    // Priority Code Point (PCP): a 3-bit field which refers to the IEEE 802.1p priority. It indicates the frame priority level.
                    // Values are from 0 (best effort) to 7 (highest); 1 represents the lowest priority.
    bool de;        // Drop Eligible (DE): a 1-bit field. May be used separately or in conjunction with PCP to indicate frames eligible to be dropped in the presence of congestion.
    short VID=0;   // VLAN Identifier (VID): a 12-bit field specifying the VLAN to which the frame belongs.
                    // The hexadecimal values of 0x000 and 0xFFF are reserved. All other values may be used as VLAN identifiers, allowing up to 4,094 VLANs.
                    // The reserved value 0x000 indicates that the frame does not belong to any VLAN; in this case,
                    // the 802.1Q tag specifies only a priority and is referred to as a priority tag. On bridges,
                    // VLAN 1 (the default VLAN ID) is often reserved for a management VLAN; this is vendor-specific.
}


// Mod1
// Ethernet 802.1ah header.
//
//
packet Ethernet1ahITag
{
    int ISid=0; // ISID
}

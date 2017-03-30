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
#include "inet/common/packet/chunk/FieldsChunk.h"
#include "inet/linklayer/common/EtherTypeTag_m.h"
#include "inet/linklayer/common/Ieee802Ctrl_m.h" // for enums
#include "inet/linklayer/common/MACAddress.h"
#include "inet/linklayer/ethernet/Ethernet.h"
}}

namespace inet;

class FieldsChunk extends cObject;

class noncobject MACAddress;

class noncobject MessageId;

enum EtherType;

enum SAPCode;

//
// Common base class for classes representing Ethernet II and 802.3 frame types,
// containing their common header fields.
//
// This class should never be instantiated by the models, only specific
// subclasses: ~EthernetIIFrame, ~EtherFrameWithLLC and ~EtherFrameWithSNAP.
//
// Source and destination MAC address are stored in data members.
// Packet length includes Etherner header, payload, padding and FCS.
// Payload length can be accessed as getEncapsulatedPacket()->getByteLength().
//
class EtherFrame extends FieldsChunk
{
    chunkLength = byte(0);
    MACAddress dest;
    MACAddress src;
}


//
// Ethernet II headers contain a 16-bit ~EtherType to identify
// the encapsulated protocol.
//
// Header length: src(6)+dest(6)+etherType(2) = 14 bytes
//
class EthernetIIFrame extends EtherFrame
{
    chunkLength = byte(ETHER_MAC_FRAME_BYTES - ETHER_FCS_BYTES);
    int etherType @enum(EtherType);
}


//
// The base Ethernet 802.3 header: contains the payload length in type-or-length field
// Do not create it!
//
class EtherFrameWithPayloadLength extends EtherFrame
{
    int payloadLength;
}


//
// Ethernet frame with 802.3 LLC header.
//
// Header length: src(6)+dest(6)+length(2)+ssap(1)+dsap(1)+control(1) = 17 bytes
//
class EtherFrameWithLLC extends EtherFrameWithPayloadLength
{
    chunkLength = byte(ETHER_MAC_FRAME_BYTES + ETHER_LLC_HEADER_LENGTH - ETHER_FCS_BYTES);
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
class EtherFrameWithSNAP extends EtherFrameWithLLC
{
    chunkLength = byte(ETHER_MAC_FRAME_BYTES + ETHER_LLC_HEADER_LENGTH + ETHER_SNAP_HEADER_LENGTH - ETHER_FCS_BYTES);
    dsap = 0xAA;
    ssap = 0xAA;
    control = 0x03;
    int orgCode;   // organizationally unique identifier (OUI); 0 for protocols that have an EtherType (ARP, IPv4, IPv6, etc.)
    int localcode; // protocol identifier (PID); stores ~EtherType if orgCode is 0
}


//
// Ethernet frame used by the PAUSE protocol
//
class EtherPauseFrame extends EthernetIIFrame
{
    chunkLength = byte(ETHER_MAC_FRAME_BYTES + ETHER_PAUSE_COMMAND_BYTES);    //FIXME Fcs?
    etherType = ETHERTYPE_FLOW_CONTROL;
    int opCode = 1; // PAUSE
    int pauseTime; // in 512 bit-time units
}

//
// Ethernet 802.1Q Tag.
//
class Ethernet1QTag extends FieldsChunk     //FIXME bad design
{
    chunkLength = byte(4);
    uint8_t pcp;    // Priority Code Point (PCP): a 3-bit field which refers to the IEEE 802.1p priority. It indicates the frame priority level.
                    // Values are from 0 (best effort) to 7 (highest); 1 represents the lowest priority.
    bool de;        // Drop Eligible (DE): a 1-bit field. May be used separately or in conjunction with PCP to indicate frames eligible to be dropped in the presence of congestion.
    short VID=0;   // VLAN Identifier (VID): a 12-bit field specifying the VLAN to which the frame belongs.
                    // The hexadecimal values of 0x000 and 0xFFF are reserved. All other values may be used as VLAN identifiers, allowing up to 4,094 VLANs.
                    // The reserved value 0x000 indicates that the frame does not belong to any VLAN; in this case,
                    // the 802.1Q tag specifies only a priority and is referred to as a priority tag. On bridges,
                    // VLAN 1 (the default VLAN ID) is often reserved for a management VLAN; this is vendor-specific.
}


//
// Ethernet 802.1ah header.
//
class Ethernet1ahITag extends FieldsChunk     //FIXME bad design
{
    chunkLength = byte(5);    //FIXME check the length value
    int ISid=0; // ISID
}

enum EthernetFcsMode {
    FCS_DECLARED_CORRECT = 1;
    FCS_DECLARED_INCORRECT = 2;
    FCS_COMPUTED = 3;
}

class EthernetPadding extends FieldsChunk
{
    chunkLength = byte(0);
}

class EthernetFcs extends FieldsChunk
{
    chunkLength = byte(ETHER_FCS_BYTES);
    int32_t fcs;
    int fcsMode @enum(EthernetFcsMode) = FCS_DECLARED_CORRECT;
}
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
#include "inet/linklayer/common/MACAddress.h"
}}
class noncobject MACAddress;
namespace inet;



//
// Message kind values used with in communication between L3 and IEEE 802 L2
//
enum Ieee802MessageKind
{
    IEEE802CTRL_DATA = 2003;  // data to/from higher layer
    IEEE802CTRL_REGISTER_DSAP = 2004;  // higher layer registers itself in LLC
    IEEE802CTRL_DEREGISTER_DSAP = 2005;  // higher layer deregisters itself in LLC
    IEEE802CTRL_SENDPAUSE = 2006;  // higher layer wants MAC to send PAUSE frame
}


//
// Some 8-bit SAP values for IEEE 802.x LLC headers.
//
enum SAPCode
{
    SAP_IBM_SNA = 0x04;
    SAP_IP = 0x06;
    SAP_3COM = 0x80;
    SAP_SNAP = 0xAA;
    SAP_BANYAN = 0xBC;
    SAP_NOVELL_IPX = 0xE0;
    SAP_LAN_MANAGER = 0xF4;
    SAP_CLNS = 0xFE;
    SAP_STP = 0x42;
}


//
// Some EtherType values (Ethernet II).
//
enum EtherType
{
    ETHERTYPE_IPv4 = 0x0800;
    ETHERTYPE_ARP = 0x0806;
    ETHERTYPE_TRILL = 0x22F3;
    ETHERTYPE_L2_ISIS = 0x22F4;
    ETHERTYPE_RARP = 0x8035;
    ETHERTYPE_IPv6 = 0x86DD;
    ETHERTYPE_INET_GENERIC = 0x86FF;    // not in standard, INET spceific value for @GenericNetworkProtocol
    ETHERTYPE_MPLS_UNICAST = 0x8847;
    ETHERTYPE_MPLS_MULTICAST = 0x8848;
}


//
// Control structure for communication between LLC and higher layers
//
class Ieee802Ctrl
{
    @customize(true);

    MACAddress src;  // src MAC address (can be left empty when sending)
    MACAddress dest; // dest MAC address
    int etherType @enum(EtherType);   // used with ~EthernetIIFrame
    int interfaceId; // incoming or outgoing interface (not needed everywhere)
    int switchPort = -1; // swich port index (only in swiches)
    int userPriority = -1; // 802.1d User Priority (UP), 0..7. Note: In 802.11, UP corresponds to 0..7 TID values, and there's a standard static mapping from TID to AC (Access Category)
    int ssap;        // used with IEEE 802 LLC (see ~EtherFrameWithLLC)
    int dsap;        // used with IEEE 802 LLC (see ~EtherFrameWithLLC)
    int pauseUnits;  // used with IEEE802CTRL_SENDPAUSE
}

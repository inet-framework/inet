//
// Copyright (C) 2003 CTIE, Monash University
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


cplusplus {{
#include "Ethernet.h"
#include "MACAddress.h"
}};

class noncobject MACAddress;


//
// Message kind values used with the Ethernet model
//
enum EtherMessageKind
{
    JAM_SIGNAL =   2000;   // jam signal on the Ethernet bus

    ETH_FRAME =    2001;   // Ethernet frame
    ETH_PAUSE =    2002;   // Ethernet PAUSE frame

    ETHCTRL_DATA =            2003;  // data to/from higher layer
    ETHCTRL_REGISTER_DSAP =   2004;  // higher layer registers itself in LLC
    ETHCTRL_DEREGISTER_DSAP = 2005;  // higher layer deregisters itself in LLC
    ETHCTRL_SENDPAUSE =       2006;  // higher layer wants MAC to send PAUSE frame
};


//
// Some EtherType values (Ethernet II).
//
enum EtherType
{
    ETHERTYPE_IP    = 0x0800;
    ETHERTYPE_ARP   = 0x0806;
    ETHERTYPE_RARP  = 0x8035;
};


//
// Some 8-bit SAP values for 802.3 LLC headers.
//
enum SAPCode
{
    SAP_IBM_SNA     = 0x04;
    SAP_IP          = 0x06;
    SAP_3COM        = 0x80;
    SAP_SNAP        = 0xAA;
    SAP_BANYAN      = 0xBC;
    SAP_NOVELL_IPX  = 0xE0;
    SAP_LAN_MANAGER = 0xF4;
    SAP_CLNS        = 0xFE;
};


//
// Control structure for communication between LLC and higher layers
//
class EtherCtrl
{
    fields:
        MACAddress src;  // used with ETHCTRL_DATA
        MACAddress dest; // used with ETHCTRL_DATA
        int etherType;   // used with ETHCTRL_DATA and EthernetIIFrame
        int ssap;        // used with ETHCTRL_DATA and EtherFrameWithLLC
        int dsap;        // used with ETHCTRL_DATA/REGISTER_DSAP/DEREGISTER_DSAP
        int pauseUnits;  // used with ETHCTRL_PAUSE
};



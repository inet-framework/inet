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

    ETHCTRL_DATA =      2003;  // data to/from higher layer
    ETHCTRL_SENDPAUSE = 2006;  // higher layer wants MAC to send PAUSE frame
};


//
// Ethernet frame.
//
// Header fields like source and destination MAC address are stored
// in message members. Other fields include:
//  - preamble: not stored (only contributes length)
//  - length: stored in cMessage::length(), not stored explicitly
//  - payload: stored a encapsulated packet (cMessage::encapsulate())
//  - crc: represented by cMessage::hasBitError()
//
message EtherFrame
{
    fields:
        MACAddress dest;
        MACAddress src;
        int control;
        int ssap;
        int dsap;
};

//
// Ethernet frame used by the PAUSE protocol
//
message EtherPauseFrame extends EtherFrame
{
    fields:
        int pauseTime; // in 512 bit-time units
}


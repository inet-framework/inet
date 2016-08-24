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
// Control structure for communication between LLC and higher layers
//
class Ieee802Ctrl
{
    int ssap;        // used with IEEE 802 LLC (see ~EtherFrameWithLLC)     TODO Ieee802SapTag
    int dsap;        // used with IEEE 802 LLC (see ~EtherFrameWithLLC)     TODO Ieee802SapTag
    int pauseUnits;  // used with IEEE802CTRL_SENDPAUSE          TODO Ieee802PauseCommand
}

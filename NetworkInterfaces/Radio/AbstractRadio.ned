//
// Copyright (C) Andras Varga, Levente Meszaros
// Based on the Mobility Framework's SnrEval by Marc Loebbers
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


//
// XXX this is a prototype...
//
// Represents the radio. Frames received from higher layers (typically the
// MAC protocol) are transmitted on the radio channel, using the global
// ChannelControl module. Frames received on the radio channel will have
// their signal-noise ratio evaluated and passed up to the decider.
// (It is the decider's task to determine whether the frame was received
// correctly, and pass it up to the MAC layer).
//
// The receive/transmit channel (frequency) can be changed by sending
// Radio a blank cMessage with PHY_C_CONFIGURERADIO as message kind, and
// a PhyControlInfo object attached to it as control info, to hold the
// new channel number. If the radio is currently transmitting, the channel
// switch will actually take place after the transmission is completed.
//
simple AbstractRadio
    gates:
        in: uppergateIn;   // from higher layer protocol (MAC)
        out: uppergateOut; // to decider (decider connects to higher layer protocol, i.e. the MAC)
        in: radioIn; // to receive frames (AirFrame) on the radio channel
endsimple


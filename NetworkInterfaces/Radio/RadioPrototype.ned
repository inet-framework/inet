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
// Prototype for radio modules.
//
// Represents the radio. Frames received from higher layers (typically the
// MAC protocol) are transmitted on the radio channel, using the global
// ChannelControl module.
//
//# TODO describe NotificationBoard stuff, how frame get received, etc...
//
// The bitrate and the receive/transmit channel can be changed by sending
// Radio a blank cMessage with PHY_C_CONFIGURERADIO as message kind, and
// a PhyControlInfo object attached to it as control info, to hold the
// new bitrate and/or channel number. If the radio is currently transmitting,
// the change will actually take place after the transmission is completed.
//
// It is also possible to specify a different bitrate for individual frames;
// to do that, attach a PhyControlInfo with birate filled in to the frame.
//
simple RadioPrototype
    gates:
        in: uppergateIn;   // from higher layer protocol (MAC)
        out: uppergateOut; // to higher layer protocol (MAC)
        in: radioIn; // to receive frames (AirFrame) on the radio channel
endsimple


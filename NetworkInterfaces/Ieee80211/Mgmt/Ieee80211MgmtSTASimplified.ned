//
// Copyright (C) 2006 Andras Varga
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
// Used in 802.11 infrastructure mode: handles management frames for a station (STA).
// Relies on the MAC layer (Ieee80211Mac) for reception and transmission of frames.
//
// This variant is a very simplistic: the AP address and channel is preconfigured,
// there is no scanning, authentication or association. Only data frames are
// understood or sent at any time.
//
// This module never switches channels, that is, it will operate on the channel
// the physical layer is configured for (see channelNumber in Ieee80211Radio).
//
// @author Andras Varga
//
simple Ieee80211MgmtSTASimplified
    parameters:
        accessPointAddress: string, // MAC address of associate AP
        frameCapacity: numeric;
    gates:
        in: uppergateIn;
        out: uppergateOut;
        in: macIn;
        out: macOut;
endsimple

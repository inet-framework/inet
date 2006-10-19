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
// Used in 802.11 infrastructure mode: in a station (STA), this module
// controls channel scanning, association and handovers, by sending commands
// (e.g. Ieee80211Prim_ScanRequest) to the management module (Ieee80211MgmtSTA).
//
// @author Andras Varga
//
simple Ieee80211AgentSTA
    parameters:
        activeScan: bool,  // selects between active and passive scanning
        channelsToScan: string,  // list of channel numbers to scan (space delimited); empty means all channels
        probeDelay: numeric const, // delay before sending a probe request during active scanning
        minChannelTime: numeric const, // min interval to spend on a channel during active scanning
        maxChannelTime: numeric const, // channel time for passive scanning, and max channel time for active scanning
        authenticationTimeout: numeric const, // timeout for the authentication procedure
        associationTimeout: numeric const; // timeout for the association procedure
    gates:
        in: mgmtIn;
        out: mgmtOut;
endsimple

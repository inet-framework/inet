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
//
// Relies on the MAC layer (Ieee80211Mac) for reception and transmission of frames.
//
// @author Andras Varga
//
simple Ieee80211MgmtSTA
    parameters:
        frameCapacity: numeric const; // maximum queue length
    gates:
        in: uppergateIn;
        out: uppergateOut;
        in: macIn;
        out: macOut;
        in: agentIn;
        out: agentOut;
endsimple

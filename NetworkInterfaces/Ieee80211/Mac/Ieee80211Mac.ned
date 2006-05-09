//
// Copyright (C) 2006 Andras Varga and Levente Mészáros
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
// Implementation of the 802.11b MAC protocol. This module is intended
// to be used in combination with the SnrEval80211 and Decider80211
// modules as physical layer.
//
// Encapsulation/decapsulation must be done in the upper layers. (It is 
// typically in the 802.11 management module, see in Ieee80211Nic).
//
// The module has an internal queue, but usually it is to be used with an
// external passive queue module (its name should be given in the queueModule
// parameter; specifying "" causes the internal queue to be used). The passive 
// queue module is a simple module whose C++ class implements the IPassiveQueue 
// interface.
//
simple Ieee80211Mac
    parameters:
        address : string,       // MAC address as hex string (12 hex digits), or
                                // "auto". "auto" values will be replaced by
                                // a generated MAC address in init stage 0.
        queueModule: string,    // name of optional external queue module
        maxQueueSize: numeric const, // max queue length in frames; only used if queueModule==""
        bitrate: numeric const;
    gates:
        in: uppergateIn;
        out: uppergateOut;
        in: lowergateIn;
        out: lowergateOut;
endsimple

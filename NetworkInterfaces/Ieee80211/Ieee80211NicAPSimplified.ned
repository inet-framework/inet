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

import
    "Ieee80211MgmtAPSimplified",
    "Ieee80211Mac",
    "Decider80211",
    "SnrEval80211";

//
// This NIC implements an infrastructure mode 802.11 network interface card
// for a wireless Access Point (AP), but using a simplified mgmt module
// that does not support handovers.
//
module Ieee80211NicAPSimplified
    gates:
        in: uppergateIn; // to upper layers
        out: uppergateOut; // from upper layers
        in: radioIn; // to receive AirFrames
    submodules:
        mgmt: Ieee80211MgmtAPSimplified;
            display: "p=96,69;i=block/cogwheel;q=wlanDataQueue";
        mac: Ieee80211Mac;
            parameters:
                queueModule = "mgmt";
            display: "p=96,155;i=block/layer";
        decider: Decider80211;
            display: "p=106,225;i=block/process_s";
        snrEval: SnrEval80211;
            display: "p=96,304;i=block/wrxtx";
    connections:
        radioIn --> snrEval.radioIn;
        decider.uppergateOut --> mac.lowergateIn display "m=m,50,50,75,0";
        snrEval.uppergateIn <-- mac.lowergateOut display "m=m,25,0,25,0";
        snrEval.uppergateOut --> decider.lowergateIn display "m=m,73,0,50,50";

        mac.uppergateOut --> mgmt.macIn;
        mac.uppergateIn <-- mgmt.macOut;

        mgmt.uppergateOut --> uppergateOut;
        mgmt.uppergateIn <-- uppergateIn;
endmodule


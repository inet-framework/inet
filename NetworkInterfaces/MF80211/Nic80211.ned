//***************************************************************************
// * file:        Nic80211.ned
// *
// * author:      Marc Loebbers
// *
// * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
// *              Technische Universitaet Berlin, Germany.
// *
// *              This program is free software; you can redistribute it
// *              and/or modify it under the terms of the GNU General Public
// *              License as published by the Free Software Foundation; either
// *              version 2 of the License, or (at your option) any later
// *              version.
// *              For further information see file COPYING
// *              in the top level directory
// ***************************************************************************
// * part of:     framework implementation developed by tkn
// **************************************************************************/

import
    "Mac80211",
    "Mac80211v2",
    "Decider80211",
    "SnrEval80211";

//
// This NIC implements an 802.11 network interface card.
//
// This component has been taken over from Mobility Framework 1.0a5.
//
// @see Mac80211, Decider80211, SnrEval80211
// @author Marc Loebbers
//
module Nic80211
    gates:
        in: uppergateIn; // to upper layers
        out: uppergateOut; // from upper layers
        in: radioIn; // to receive AirFrames
    submodules:
        mac: Mac80211;
            display: "p=96,87;i=block/layer";
        decider: Decider80211;
            display: "p=106,157;i=block/process_s";
        snrEval: SnrEval80211;
            display: "p=96,236;i=block/wrxtx";
    connections:
        radioIn --> snrEval.radioIn;
        decider.uppergateOut --> mac.lowergateIn display "m=m,50,50,75,0";
        snrEval.uppergateIn <-- mac.lowergateOut display "m=m,25,0,25,0";
        snrEval.uppergateOut --> decider.lowergateIn display "m=m,73,0,50,50";

        mac.uppergateOut --> uppergateOut;
        mac.uppergateIn <-- uppergateIn;
endmodule


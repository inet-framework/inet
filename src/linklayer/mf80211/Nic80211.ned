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


package inet.linklayer.mf80211;

import inet.linklayer.mf80211.phyLayer.snrEval.SnrEval80211;
import inet.linklayer.mf80211.phyLayer.decider.Decider80211;
import inet.linklayer.mf80211.macLayer.Mac80211;
import inet.linklayer.IWirelessNic;


//
// This NIC implements an 802.11 network interface card.
//
// This component has been taken over from Mobility Framework 1.0a5.
//
// @see ~Mac80211, ~Decider80211, ~SnrEval80211
// @author Marc Loebbers
//
module Nic80211 like IWirelessNic
{
    parameters:
        @display("i=block/ifcard");
        string mgmtType = "";      // unused
    gates:
        input upperLayerIn @labels(Ieee802Ctrl); // to upper layers
        output upperLayerOut; // from upper layers
        input radioIn @labels(AirFrame); // to receive AirFrames
    submodules:
        mac: Mac80211 {
            parameters:
                @display("p=96,87");
        }
        decider: Decider80211 {
            parameters:
                @display("p=106,157");
        }
        snrEval: SnrEval80211 {
            parameters:
                @display("p=96,236");
        }
    connections:
        radioIn --> snrEval.radioIn;
        decider.upperLayerOut --> { @display("m=m,50,50,75,0"); } --> mac.lowerLayerIn;
        snrEval.upperLayerIn <-- { @display("m=m,25,0,25,0"); } <-- mac.lowerLayerOut;
        snrEval.upperLayerOut --> { @display("m=m,73,0,50,50"); } --> decider.lowerLayerIn;

        mac.upperLayerOut --> upperLayerOut;
        mac.upperLayerIn <-- upperLayerIn;
}


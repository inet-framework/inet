//***************************************************************************
// * file:        NicCsma.ned
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

import inet.linklayer.mf80211.macLayer.CSMAMacLayer;
import inet.linklayer.mf80211.phyLayer.snrEval.SnrEval;
import inet.linklayer.mfcore.BasicDecider;


//
// The CsmaNic contains a csma-like MAC layer.
// The SnrEval module is used as the mac needs a RadioState to be published.
// However as still the BasicDecider is used all messages will be delivered to
// the MAC and no packets get lost
//
// This nic is intended to be used in order to test the implementation
// of the csma protocol.
//
// IMPORTANT:<br> In order to work with the ChannelControl module the snrEval
// module has to be called "snrEval" in the ned file.
//
// This component has been taken over from Mobility Framework 1.0a5.
//
// @see BasicMacLayer, BasicDecider, BasicSnrEval
// @author Marc Loebbers
//
module NicCsma
{
    gates:
        input uppergateIn; // to upper layers
        output uppergateOut; // from upper layers
        input radioIn @labels(AirFrame); // to receive AirFrames
    submodules:
        mac: CSMAMacLayer {
            parameters:
                @display("p=96,87");
        }
        decider: BasicDecider {
            parameters:
                @display("p=106,157");
        }
        snrEval: SnrEval {
            parameters:
                @display("p=96,236");
        }
    connections:
        radioIn --> snrEval.radioIn;
        decider.uppergateOut --> { @display("m=m,50,50,75,0"); } --> mac.lowergateIn;
        snrEval.uppergateIn <-- { @display("m=m,25,0,25,0"); } <-- mac.lowergateOut;
        snrEval.uppergateOut --> { @display("m=m,73,0,50,50"); } --> decider.lowergateIn;

        mac.uppergateOut --> uppergateOut;
        mac.uppergateIn <-- uppergateIn;
}

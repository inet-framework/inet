//***************************************************************************
// * file:        Decider80211.ned
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


package inet.linklayer.mf80211.phyLayer.decider;

//
// Decider module of the 802.11 modules
//
// This component has been taken over from Mobility Framework 1.0a5.
//
simple Decider80211
{
    parameters:
        bool debug = default(false); // debug switch
        double snirThreshold @unit("dB") = default(4dB);
        double bitrate @unit("bps");
        @display("i=block/process_s");
    gates:
        output uppergateOut @labels(Mac80211Pkt);
        input lowergateIn;
}

//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac;

import inet.common.SimpleModule;
import inet.linklayer.ieee80211.mac.contract.IRx;

//
// Rx checks received frames for errors and manages received NAV and shared TXNAV.
// Successful HCF holder frames replace TXNAV from the PPDU end and encoded Duration/ID.
// TXNAV blocks contention until expiry, including after TXOP end.
// HCF also disables local contention through its active sequence and response waits.
// The medium query for CTS responses uses radio state and received NAV only.
//
// This is the default implementation of the ~IRx module interface.
//
simple Rx extends SimpleModule like IRx
{
    parameters:
        @class(Rx);
        displayStringTextFormat = default("{rxStatus}");
        @display("i=block/rx");
        @signal[navChanged](type=simtime_t);
        @signal[packetDropped](type=inet::Packet);
        @statistic[nav](title="network allocation vector"; source=navChanged; record=vector; interpolationmode=linear);
}

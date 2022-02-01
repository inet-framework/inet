//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.originator;

simple TxopProcedure
{
    parameters:
        double txopLimit @unit(s) = default(-1s);
        @display("i=block/timer");
        @signal[txopStarted](inet::ieee80211::TxopProcedure);
        @signal[txopEnded](inet::ieee80211::TxopProcedure);
        @statistic[txopEnded](title="txops ended"; source=txopEnded; record=count,vector?; interpolationmode=none);
        @statistic[txopDuration](title="txop duration"; source=txopDuration(txopEnded); record=histogram,vector?; interpolationmode=none);
        @statistic[txopActive](title="txop active"; source=warmup(count(txopStarted)-count(txopEnded)); record=vector; interpolationmode=sample-hold; autoWarmupFilter=false);
}


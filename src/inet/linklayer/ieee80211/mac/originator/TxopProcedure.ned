//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.originator;

import inet.common.SimpleModule;

//
// Implements the Transmission Opportunity (TXOP) mechanism in IEEE 802.11 QoS
// networks. Manages TXOP periods during which a station has the right to transmit. 
// Tracks TXOP limits based on access category (AC) and PHY mode. Supports both single
// and multiple protection mechanisms to protect frame exchanges during the TXOP.
//
simple TxopProcedure extends SimpleModule
{
    parameters:
        @class(TxopProcedure);
        double txopLimit @unit(s) = default(-1s);
        @display("i=block/timer");
        @signal[txopStarted](inet::ieee80211::TxopProcedure);
        @signal[txopEnded](inet::ieee80211::TxopProcedure);
        @statistic[txopEnded](title="txops ended"; source=txopEnded; record=count,vector?; interpolationmode=none);
        @statistic[txopDuration](title="txop duration"; source=txopDuration(txopEnded); record=histogram,vector?; interpolationmode=none);
        @statistic[txopActive](title="txop active"; source=warmup(count(txopStarted)-count(txopEnded)); record=vector; interpolationmode=sample-hold; autoWarmupFilter=false);
}


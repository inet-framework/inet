//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.originator;

//
// Implements the Transmission Opportunity (TXOP) mechanism in IEEE 802.11e/n/ac networks. 
// TXOP allows a station to transmit multiple frames in succession without contending 
// for the channel between each frame, improving efficiency especially for QoS traffic. 
// This module manages the TXOP duration, ensuring that transmissions do not exceed the 
// configured TXOP limit for each access category. It tracks active TXOPs, signals their 
// start and end events, and collects statistics on TXOP usage. The default txopLimit 
// of -1s indicates no limit (used for legacy DCF operation), while positive values 
// enable TXOP functionality with the specified maximum duration.
//
// @see ~Edcaf, ~Hcf
//
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

//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.ratecontrol;

import inet.common.SimpleModule;
import inet.linklayer.ieee80211.mac.contract.IRateControl;

//
// Implements Atsushi Onoe's credit-based rate adaptation rules, following
// MadWifi ath_rate/onoe/onoe.c (revision a7531fd223a1f454d3fd74a975b4581cde5411bb).
// Evaluates completed normal-ACK frames after the interval, including the
// completion that reaches the deadline. Insufficient samples are retained;
// upward credit requires at least ten completions and no terminal failures.
// A peer's first interval starts when it is first seen. Rate queries do not
// trigger evaluation. An explicit initialRate overrides automatic startup.
// Automatic startup uses the fastest local 11b mode, or the fastest local
// OFDM/ERP mode at or below 36 Mbps (the slowest mode if none qualifies).
// These are local modes, not negotiated peer rates. HT/VHT retain the fastest
// mandatory startup mode. Retries use the stored rate.
// Failed RTS/CTS exchanges contribute to the completed packet's retry total;
// RTS exhaustion counts as one terminal error. As an INET modeling choice,
// internal-collision exhaustion also counts as one terminal error, with the
// packet's full recovery count (including internal collisions).
// MadWifi's multirate retry series
// is not modeled. Block Ack feedback equivalence and HT-or-newer adaptation
// fidelity are not established.
//
simple OnoeRateControl extends SimpleModule like IRateControl
{
    parameters:
        @class(OnoeRateControl);
        double initialRate @unit(bps) = default(-1bps); // -1 selects automatic startup as described above
        double interval @unit(s) = default(1s);
        displayStringTextFormat = default("{numStations} stations");
        @display("i=block/cogwheel");
        @signal[datarateChanged];
        @statistic[datarateChanged](title="datarate"; record=vector; interpolationmode=sample-hold); // aggregate rate over all stations (interleaved)
        @statistic[dataratePerStation](title="data rate per station"; source=demux(datarateChanged); unit=bps; record=vector; interpolationmode=sample-hold); // one vector per receiver, demultiplexed by station
}

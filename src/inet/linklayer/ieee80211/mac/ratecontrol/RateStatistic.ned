//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.ratecontrol;

import inet.common.SimpleModule;

//
// Records the IEEE 802.11 data rate a rate control algorithm currently uses
// towards one particular peer station. The rate control module (see
// ~RateControlBase, e.g. ~AarfRateControl or ~OnoeRateControl) creates one
// such submodule dynamically per receiver when its `recordPerStationRate`
// parameter is enabled, and updates it whenever the rate to that station
// changes. Because each peer gets its own submodule, the data rate to each
// station becomes a separate, per-station statistic that can be plotted in the
// Analysis tool - unlike the aggregate `datarateChanged` statistic on the rate
// control module, which interleaves all stations into one vector.
//
// See the corresponding C++ class for more details.
//
simple RateStatistic extends SimpleModule
{
    parameters:
        @class(RateStatistic);
        @display("i=block/timer");
        @signal[datarateChanged](type=double);
        @statistic[datarateChanged](title="data rate to peer"; unit=bps; record=vector; interpolationmode=sample-hold);
}

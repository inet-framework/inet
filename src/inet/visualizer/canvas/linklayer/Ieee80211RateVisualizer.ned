//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.linklayer;

import inet.visualizer.canvas.common.StatisticCanvasVisualizer;

//
// Displays a small per-station data rate bar chart above each IEEE 802.11
// access point that runs an adaptive rate control (~AarfRateControl,
// ~OnoeRateControl). It is a thin configuration of the generic
// ~StatisticCanvasVisualizer bar chart mode: it subscribes to the rate
// control's `datarateChanged` signal, which tags each value with the receiver
// as a named details object, and draws one bar per station (the bar height is
// proportional to the PHY data rate to that station, clamped to `maxRate`).
//
// In its default configuration it is driven by the rate control's signal, so it
// only shows rates where an adaptive rate control is configured. To visualize
// interface-wide fixed rates or per-receiver configured rates
// (`dataFrameBitratePerReceiver`) instead, point it at the coordination
// function's `datarateSelected` signal, which reports the rate actually selected
// for every transmitted frame and is likewise tagged per receiver:
//
// <pre>
// *.visualizer.*.rateVisualizer.displayRates = true
// *.visualizer.*.rateVisualizer.signalName = "datarateSelected"
// *.visualizer.*.rateVisualizer.sourceFilter = "*.accessPoint.wlan[*].mac.dcf"
// </pre>
//
// In that mode a station's bar appears when the access point first sends it a
// data frame, rather than as soon as its rate is configured.
//
// @see ~StatisticCanvasVisualizer, ~StatisticVisualizerBase, ~AarfRateControl
//
simple Ieee80211RateVisualizer extends StatisticCanvasVisualizer
{
    parameters:
        bool displayRates = default(false); // Display the per-station data rate bar chart above access points
        displayStatistics = default(this.displayRates);

        string nodeFilter = default("*"); // Which access point nodes' rate controls are shown
        sourceFilter = default("*." + this.nodeFilter + ".**.rateControl"); // Match the rate control modules under the selected nodes

        double maxRate @unit(bps) = default(54Mbps); // Data rate mapped to a full-height bar and to the fast end of the color gradient
        maxValue = default(this.maxRate / 1Mbps); // The generic bar chart scales in the display unit (Mbps)

        // Fixed rate-visualizer configuration of the generic statistic bar chart:
        signalName = default("datarateChanged");
        displayMode = default("bars");
        statisticUnit = default("bps");
        unit = default("Mbps");
        valueFormat = default("%.0f"); // Rate label rounded to the nearest Mbps
        barColor = default("red orange yellow lime green"); // Slow (first) to fast (last) rate gradient
        nameRotation = default(45deg);
        zIndex = default(2);
}

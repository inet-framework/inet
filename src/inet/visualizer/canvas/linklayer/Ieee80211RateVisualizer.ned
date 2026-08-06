//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.linklayer;

import inet.visualizer.canvas.common.StatisticCanvasVisualizer;

//
// Displays a small per-station data rate bar chart above each IEEE 802.11
// access point that runs an adaptive rate control (~AarfRateControl,
// ~OnoeRateControl). It is a configuration of the generic
// ~StatisticCanvasVisualizer rather than new visualizer code: it subscribes to
// the rate control's `datarateChanged` signal, which tags each value with the
// receiver as a named details object, displays one bar per receiver, and
// configures a bar chart figure with the scale, colors and label format that
// suit a data rate.
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

        // Fixed rate visualizer configuration of the generic statistic visualizer:
        signalName = default("datarateChanged");
        splitBy = default("details"); // One statistic per receiver, as tagged on the signal
        groupBy = default("source"); // All receivers of one network interface on one bar chart
        statisticUnit = default("bps");
        unit = default("Mbps");
        figure = default({type: "barChart",
                          maxValue: this.maxRate / 1Mbps, // the values are displayed in Mbps
                          barColor: "red orange yellow lime green", // slow (first) to fast (last) rate gradient
                          valueFormat: "%.0f", // rate rounded to the nearest Mbps
                          title: "rate [Mbps]"});
        zIndex = default(2);
}


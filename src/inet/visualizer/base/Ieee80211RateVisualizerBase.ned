//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// Base module for IEEE 802.11 per-station data rate visualizer simple modules.
//
// Above each IEEE 802.11 access point it displays a small bar chart, with one
// bar per station. The height of each bar is proportional to the PHY data rate
// the access point uses towards that station (clamped to `maxRate`), and the
// rate is annotated on top of the bar. Station names are the x-axis labels,
// rotated by `nameRotation` degrees so the chart stays narrow. The bar color is
// interpolated on a rate-dependent gradient (fast rates near the last color,
// slow rates near the first color).
//
// A bar exists for a station S under an access point A if and only if A has
// transmitted a unicast data frame to S, or A has a configured per-receiver
// data rate for S (via the `dataFrameBitratePerReceiver` rate selection
// parameter). No association tracking is performed. The displayed rate is the
// observed rate of the last data frame transmitted to S, or the configured rate
// if A has not transmitted to S yet.
//
// This module subscribes to the datarateSelected signal on the module
// determined by the `visualizationSubjectModule` parameter.
//
// @see ~Ieee80211RateCanvasVisualizer, ~Ieee80211RateVisualizer, ~IIeee80211RateVisualizer, ~VisualizerBase
//
simple Ieee80211RateVisualizerBase extends VisualizerBase
{
    parameters:
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // Module path to the network node visualizer module, refers to a sibling submodule by default

        bool displayRates = default(false); // Display per-station data rate bar chart above access points, disabled by default

        string nodeFilter @mutable = default("*"); // Which access point nodes are considered, matches all nodes by default
        string interfaceFilter @mutable = default("*"); // Which access point interfaces are considered, matches all interfaces by default

        double maxRate @unit(bps) = default(54Mbps); // Data rate mapped to the full bar height and to the fast end of the color gradient

        double barWidth = default(8); // Width of each bar in pixels
        double barSpacing = default(6); // Horizontal spacing between adjacent bars in pixels
        double maxBarHeight = default(40); // Height of a full (>= maxRate) bar in pixels

        string rateColor = default("red orange yellow lime green"); // Rate to color gradient from slow (first) to fast (last), a list of colors or a color group name
        string rateFormat = default("%r"); // Rate label format: %r prints the rate rounded to the nearest Mbps with an "M" suffix (e.g. "54M"), %g prints the rate in bps

        string rateLabelFont = default("<default>,6"); // Rate label font, small by default
        string rateLabelColor = default("black"); // Rate label color, black by default
        string stationLabelFont = default("<default>,6"); // Station name label font, small by default
        string stationLabelColor = default("black"); // Station name label color, black by default
        double nameRotation @unit(deg) = default(45deg); // Rotation angle of the station name labels, 45 degrees by default

        bool displayTitle = default(true); // Display a small title (the access point interface name) above the chart
        string titleFont = default("<default>,7"); // Chart title font
        string titleColor = default("black"); // Chart title color

        double holdTime @unit(s) = default(0s); // How long an observed rate bar is kept after the last data frame to the station, 0 means forever

        string placementHint = default("top"); // Annotation placement hint, space-separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double placementPriority = default(0); // Determines the order of annotation positioning

        @class(Ieee80211RateVisualizerBase);
}

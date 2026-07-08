//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.linklayer;

import inet.visualizer.base.Ieee80211RateVisualizerBase;
import inet.visualizer.contract.IIeee80211RateVisualizer;

//
// Visualizes IEEE 802.11 per-station data rates on a 2D canvas. Above each
// access point it displays a small bar chart, one bar per station, with the bar
// height proportional to the PHY data rate the access point uses towards that
// station. The rate is annotated on top of the bar, and the (rotated) station
// name labels the x-axis.
//
// @see ~Ieee80211RateVisualizer, ~Ieee80211RateVisualizerBase, ~IIeee80211RateVisualizer
//
simple Ieee80211RateCanvasVisualizer extends Ieee80211RateVisualizerBase like IIeee80211RateVisualizer
{
    parameters:
        double zIndex = default(2); // Determines the drawing order of figures relative to other visualizers
        @class(Ieee80211RateCanvasVisualizer);
}

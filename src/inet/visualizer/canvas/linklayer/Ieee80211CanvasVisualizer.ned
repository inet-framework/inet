//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.linklayer;

import inet.visualizer.base.Ieee80211VisualizerBase;
import inet.visualizer.contract.IIeee80211Visualizer;

//
// This module visualizes IEEE 802.11 associations on a 2D canvas. It displays
// an icon labelled with the SSID for each associated wireless interface.
//
// @see ~Ieee80211OsgVisualizer, ~Ieee80211Visualizer, ~Ieee80211VisualizerBase, ~IIeee80211Visualizer
//
simple Ieee80211CanvasVisualizer extends Ieee80211VisualizerBase like IIeee80211Visualizer
{
    parameters:
        double zIndex = default(2); // determines the drawing order of figures relative to other visualizers
        @class(Ieee80211CanvasVisualizer);
}


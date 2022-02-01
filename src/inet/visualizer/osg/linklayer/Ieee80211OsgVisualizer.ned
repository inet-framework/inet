//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.linklayer;

import inet.visualizer.base.Ieee80211VisualizerBase;
import inet.visualizer.contract.IIeee80211Visualizer;

//
// This module visualizes IEEE 802.11 associations on a 3D osg scene. It displays
// an icon labelled with the SSID for each associated wireless interface.
//
// @see ~Ieee80211CanvasVisualizer, ~Ieee80211Visualizer, ~Ieee80211VisualizerBase, ~IIeee80211Visualizer
//
simple Ieee80211OsgVisualizer extends Ieee80211VisualizerBase like IIeee80211Visualizer
{
    parameters:
        @class(Ieee80211OsgVisualizer);
}


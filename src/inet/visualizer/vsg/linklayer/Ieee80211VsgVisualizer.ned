//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.vsg.linklayer;

import inet.visualizer.base.Ieee80211VisualizerBase;
import inet.visualizer.contract.IIeee80211Visualizer;

//
// Visualizes IEEE 802.11 associations in a 3D VulkanSceneGraph (VSG) scene.
// Displays a colored sphere and SSID label annotation on each node that has an
// active wireless association.
//
// The OSG twin (~Ieee80211OsgVisualizer) renders a textured-quad billboard
// loaded from the icon image; this VSG implementation approximates that with a
// colored sphere (color derived from the SSID) and a text label because VsgUtils
// does not yet expose a textured-billboard helper.
//
// @see ~Ieee80211CanvasVisualizer, ~Ieee80211OsgVisualizer, ~Ieee80211Visualizer, ~Ieee80211VisualizerBase, ~IIeee80211Visualizer
//
simple Ieee80211VsgVisualizer extends Ieee80211VisualizerBase like IIeee80211Visualizer
{
    parameters:
        @class(Ieee80211VsgVisualizer);
}

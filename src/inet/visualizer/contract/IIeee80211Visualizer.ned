//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for IEEE 802.11 visualizers. IEEE 802.11 visualizers display
// wireless network associations between access points and stations.
//
// @see ~Ieee80211CanvasVisualizer, ~Ieee80211OsgVisualizer, ~Ieee80211Visualizer, ~Ieee80211VisualizerBase
//
moduleinterface IIeee80211Visualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}

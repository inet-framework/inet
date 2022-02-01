//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.common;

import inet.visualizer.contract.IIeee80211Visualizer;

//
// This module integrates canvas and osg IEEE 802.11 association visualizers into a single module.
//
// @see ~Ieee80211CanvasVisualizer, ~Ieee80211OsgVisualizer, ~Ieee80211VisualizerBase, ~IIeee80211Visualizer
//
module Ieee80211Visualizer like IIeee80211Visualizer
{
    parameters:
        @display("i=block/app");
        *.visualizationTargetModule = default("^.^");
        *.visualizationSubjectModule = default("^.^");
    submodules:
        canvasVisualizer: <default(firstAvailableOrEmpty("Ieee80211CanvasVisualizer"))> like IIeee80211Visualizer if typename != "" {
            parameters:
                @display("p=100,100");
        }
        osgVisualizer: <default(firstAvailableOrEmpty("Ieee80211OsgVisualizer"))> like IIeee80211Visualizer if typename != "" {
            parameters:
                @display("p=100,200");
        }
}


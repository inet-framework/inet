//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.common;

import inet.visualizer.contract.IIeee80211RateVisualizer;

//
// Integrates canvas (and, when available, osg) IEEE 802.11 per-station data
// rate visualizers into a single module. Only a canvas implementation is
// provided; the osg slot is left empty.
//
// @see ~Ieee80211RateCanvasVisualizer, ~Ieee80211RateVisualizerBase, ~IIeee80211RateVisualizer
//
module Ieee80211RateVisualizer like IIeee80211RateVisualizer
{
    parameters:
        @display("i=block/app");
        string visualizationTargetModule = default("^");
        string visualizationSubjectModule = default("^");
        *.visualizationTargetModule = default(absPath(this.visualizationTargetModule));
        *.visualizationSubjectModule = default(absPath(this.visualizationSubjectModule));
    submodules:
        canvasVisualizer: <default(firstAvailableOrEmpty("Ieee80211RateCanvasVisualizer"))> like IIeee80211RateVisualizer if typename != "" {
            parameters:
                @display("p=100,100");
        }
        osgVisualizer: <default("")> like IIeee80211RateVisualizer if typename != "" {
            parameters:
                @display("p=100,200");
        }
}

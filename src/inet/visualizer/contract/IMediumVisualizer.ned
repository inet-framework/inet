//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for medium visualizers. Medium visualizers display radio signal
// propagation, transmission and reception activities in wireless networks.
//
// @see ~MediumCanvasVisualizer, ~MediumOsgVisualizer, ~MediumVisualizer, ~MediumVisualizerBase
//
moduleinterface IMediumVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}

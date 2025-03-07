//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for mobility visualizers. Mobility visualizers display movement
// patterns and trajectories of network nodes based on their mobility models.
//
// @see ~MobilityCanvasVisualizer, ~MobilityOsgVisualizer, ~MobilityVisualizer, ~MobilityVisualizerBase
//
moduleinterface IMobilityVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}

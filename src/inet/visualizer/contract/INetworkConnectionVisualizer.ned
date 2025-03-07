//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for network connection visualizers. Network connection visualizers
// create visual representations of connections between network nodes in the
// simulation.
//
// @see ~NetworkConnectionCanvasVisualizer, ~NetworkConnectionOsgVisualizer, ~NetworkConnectionVisualizer, ~NetworkConnectionVisualizerBase
//
moduleinterface INetworkConnectionVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}

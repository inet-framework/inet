//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// Base module for node visualizer simple modules. The actual
// visualization of network nodes is done in derived modules.
//
// @see ~NetworkNodeCanvasVisualizer, ~NetworkNodeOsgVisualizer, ~INetworkNodeVisualizer, ~VisualizerBase
//
simple NetworkNodeVisualizerBase extends VisualizerBase
{
    parameters:
        string nodeFilter @mutable = default("*"); // Determines which network nodes are displayed. All nodes are displayed by default.

        double annotationSpacing = default(4);
        double placementPenalty = default(10);

        @class(NetworkNodeVisualizerBase);
}


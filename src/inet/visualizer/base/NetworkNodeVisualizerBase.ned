//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for node visualizer simple modules. The actual
// visualization of network nodes is done in derived modules.
//
// @see ~NetworkNodeCanvasVisualizer, ~NetworkNodeOsgVisualizer, ~INetworkNodeVisualizer, ~VisualizerBase
//
simple NetworkNodeVisualizerBase extends VisualizerBase
{
    parameters:
        string nodeFilter @mutable = default("*"); // determines which network nodes are displayed, all nodes by default

        double annotationSpacing = default(4);
        double placementPenalty = default(10);

        @class(NetworkNodeVisualizerBase);
}


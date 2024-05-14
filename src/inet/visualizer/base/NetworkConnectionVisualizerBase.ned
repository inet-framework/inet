//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for connection visualizer simple modules. The actual
// visualization of network connections is done in derived modules.
//
// @see ~NetworkConnectionCanvasVisualizer, ~NetworkConnectionOsgVisualizer, ~INetworkConnectionVisualizer, ~VisualizerBase
//
simple NetworkConnectionVisualizerBase extends VisualizerBase
{
    parameters:
        bool displayNetworkConnections = default(true);

        string nodeFilter @mutable = default("*"); // Determines which network connections are displayed. All connections are displayed by default.

        string lineColor = default("black"); // Connection line color.
        string lineStyle = default("solid"); // Connection line style (solid, dashed, dotted).
        double lineWidth = default(1); // Connection line width.

        @class(NetworkConnectionVisualizerBase);
}


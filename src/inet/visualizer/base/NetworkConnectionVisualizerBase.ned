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

        string nodeFilter @mutable = default("*"); // determines which network connections are displayed, all connections by default

        string lineColor = default("black"); // connection line color
        string lineStyle = default("solid"); // connection line style (solid, dashed, dotted)
        double lineWidth = default(1); // connection line width

        @class(NetworkConnectionVisualizerBase);
}


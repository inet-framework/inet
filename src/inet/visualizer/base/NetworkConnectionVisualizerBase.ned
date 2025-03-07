//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for connection visualizer simple modules. It visualizes
// physical connections between network nodes, such as wired links, as lines
// in the simulation. TODO what is this
//
// By default, network connections are displayed. To disable visualization or
// filter which connections are shown, use the displayNetworkConnections and
// nodeFilter parameters.
//
// The visualizer shows connections as straight lines between network nodes,
// with configurable color, style, and width. This helps understand the network
// topology and physical connectivity between devices.
//
// The actual rendering of network connections is implemented in derived
// modules for specific user interfaces (canvas or OSG).
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

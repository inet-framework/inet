//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for transport connection visualizer simple modules. It
// visualizes active transport layer connections (such as TCP sessions) between
// network nodes, showing the endpoints of established connections.
//
// By default, transport connections are not displayed. To enable visualization,
// set the displayTransportConnections parameter to true and use the filters to
// select which source/destination nodes and ports to monitor.
//
// The visualizer shows transport connections as marker icons at the endpoints
// of the connection, with optional labels showing connection details. It tracks
// the establishment and termination of connections, updating the visualization
// as connections are created and closed.
//
// This module subscribes to `tcpConnectionAdded` and `tcpConnectionRemoved` signals
// on the module determined by the `visualizationSubjectModule` parameter. When a
// connection is established or terminated, the visualizer updates its display
// accordingly.
//
// This visualizer is particularly useful for understanding application communication
// patterns, analyzing client-server interactions, debugging connection establishment
// issues, and monitoring the overall connection state of the network.
//
// The actual rendering of transport connection visualizations is implemented in
// derived modules for specific user interfaces (canvas or OSG).
//
// @see ~TransportConnectionCanvasVisualizer, ~TransportConnectionOsgVisualizer, ~ITransportConnectionVisualizer, ~VisualizerBase
//
simple TransportConnectionVisualizerBase extends VisualizerBase
{
    parameters:
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // The network node visualizer module, refers to a sibling submodule by default

        bool displayTransportConnections = default(false);// display transport connection endpoints, disabled by default

        string sourceNodeFilter @mutable = default("*"); // Which source network nodes are considered to display connections, matches no nodes by default
        string sourcePortFilter @mutable = default("*"); // Which source ports are considered to display connections, matches all ports by default
        string destinationNodeFilter @mutable = default("*"); // Which destination network nodes are considered to display connections, matches no nodes by default
        string destinationPortFilter @mutable = default("*"); // Which destination ports are considered to display connections, matches all ports by default

        string icon = default("misc/marker_s"); // Transport connection visualization icon
        string iconColor = default("dark"); // Transport connection icon color is a list of colors or a color group name (e.g. dark, light), a set of dark colors by default
        string labelFont = default("<default>"); // Label font, automatic by default
        string labelColor = default("black"); // Label color, black by default
        string placementHint = default("top"); // Annotation placement hint, space separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double placementPriority = default(0); // Determines the order of annotation positioning

        @class(TransportConnectionVisualizerBase);
}

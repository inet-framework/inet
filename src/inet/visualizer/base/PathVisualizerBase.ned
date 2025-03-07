//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for path visualizer simple modules. It visualizes
// the routes that packets take through the network, showing end-to-end paths
// and intermediate hops as animated polyline arrows.
//
// By default, routes are not displayed. To enable visualization, set the
// displayRoutes parameter to true and use the filters to select which
// nodes and packets to monitor.
//
// The visualizer tracks packet paths by monitoring protocol modules at different
// layers of the network stack. It works by:
// - Detecting when packets enter the network stack from applications (packetReceivedFromUpper)
// - Tracking packets as they pass through intermediate nodes (packetReceivedFromLower)
// - Identifying when packets reach their final destination (packetSentToUpper)
//
// When a complete path is detected, the visualizer draws a polyline arrow
// connecting the source, intermediate nodes, and destination. These paths
// fade out after a configurable amount of time unless reinforced by new packets
// following the same route.
//
// The path visualization includes:
// - Colored arrows showing the complete route from source to destination
// - Automatic shifting of overlapping paths for better visibility
// - Optional labels with packet information
// - Smooth animation and fade-out effects
//
// This visualizer is particularly useful for understanding routing behavior,
// analyzing network traffic patterns, debugging protocol implementations,
// and demonstrating how packets traverse the network.
//
// The format string can contain the following directives:
//  - %n packet name
//  - %c packet class
//
// @see ~NetworkRouteCanvasVisualizer, ~NetworkRouteOsgVisualizer, ~INetworkRouteVisualizer,
// ~TransportRouteCanvasVisualizer, ~TransportRouteOsgVisualizer, ~ITransportRouteVisualizer
//
simple PathVisualizerBase extends VisualizerBase
{
    parameters:
        bool displayRoutes = default(false); // Display polyline arrow for active routes, disabled by default

        string startPathSignal = default("packetReceivedFromUpper");
        string extendPathSignal = default("packetReceivedFromLower");
        string endPathSignal = default("packetSentToUpper");

        string nodeFilter @mutable = default("*"); // Which nodes are considered, matches all nodes by default
        object packetFilter @mutable = default("*"); // Which packets are considered, matches all packets by default

        string lineColor = default("dark"); // Route color is a list of colors or a color group name (e.g. dark, light), a set of dark colors by default
        string lineStyle = default("solid"); // Route line style (solid, dashed, dotted)
        double lineWidth = default(3); // Route line width
        bool lineSmooth = default(false); // When true polylines are displayed as curves

        double lineShift = default(16); // Route line shift to avoid overlapping routes
        string lineShiftMode = default("normal"); // Determines how overlapping lines are shifted, possible values are: normal, x, y, z; optional prefix + or -

        double lineContactSpacing = default(3); // Spacing between arrow end and submodule icon
        string lineContactMode @enum("circular", "rectangular") = default("rectangular"); // Determines how arrows are clipped at the submodules

        string labelFormat = default("%n"); // Determines what path data is displayed
        string labelFont = default("<default>, 8px"); // Label font, automatic by default
        string labelColor = default(""); // Label color, same as line by default

        string fadeOutMode @enum("realTime", "animationTime", "simulationTime") = default("realTime"); // Specifies how inactive routes fade out
        double fadeOutTime @unit(s) = default(1s); // How quickly inactive routes fade away, 1 second by default
        double fadeOutAnimationSpeed = default(0); // Animation speed while there are active routes, value must be in the range (0, +inf)

        @class(PathVisualizerBase);
}

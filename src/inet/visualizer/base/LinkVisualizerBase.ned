//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for link visualizer simple modules. It provides the
// foundation for visualizing active communication links between network nodes
// at various protocol layers (physical, data link, network, etc.).
//
// By default, links are not displayed. To enable visualization, set the
// displayLinks parameter to true in the derived visualizer module and use
// the filters to select which nodes, interfaces, and packets to monitor.
//
// The visualizer shows active links as arrows between network nodes, with
// optional labels showing packet information. A link is considered active
// when a packet successfully travels from one node to another at the
// corresponding protocol layer. Links fade out after a configurable amount
// of time if no new packets are transmitted.
//
// This module subscribes to signals at the provided subscription module and
// tracks packet transmissions between nodes. When determining active links,
// it only considers packets matched by the provided filters. The filters use
// the format of `cMatchExpression` with the packet name as the default attribute.
//
// The activityLevel parameter determines what level of communication is visualized:
// - service: visualizes application-level communication (end-to-end)
// - peer: visualizes protocol-level communication (peer-to-peer)
// - protocol: visualizes all protocol-internal communication
//
// This base module is extended by specific link visualizers for different
// protocol layers, such as PhysicalLinkVisualizer for physical layer links
// and DataLinkVisualizer for data link layer connections.
//
// The format string can contain the following directives:
//  - %n packet name
//  - %c packet class
//
// @see ~PhysicalLinkVisualizerBase, ~DataLinkVisualizerBase, ~VisualizerBase
//
simple LinkVisualizerBase extends VisualizerBase
{
    parameters:
        bool displayLinks = default(false); // Display arrows for active links, disabled by default
        string activityLevel @enum("service", "peer", "protocol") = default("service"); // Determines what level of activity is displayed, service level by default

        string nodeFilter @mutable = default("*"); // Which nodes are considered, matches all nodes by default
        string interfaceFilter @mutable = default("*"); // Which interfaces are considered, matches all interfaces by default
        object packetFilter @mutable = default("*"); // Which packets are considered, matches all packets by default

        string lineColor = default("black"); // Link arrow color, black by default
        string lineStyle = default("solid"); // Link arrow style (solid, dashed, dotted)
        double lineWidth = default(3); // Link arrow width

        double lineShift = default(16); // Line shift to avoid overlapping arrows
        string lineShiftMode = default("normal"); // Determines how overlapping arrows are shifted, possible values are: none, normal, x, y, z; optional prefix + or -

        double lineContactSpacing = default(3); // Spacing between arrow end and submodule icon
        string lineContactMode @enum("circular", "rectangular") = default("rectangular"); // Determines how arrows are clipped at the submodules

        string labelFormat = default("%n"); // Determines what link data is displayed
        string labelFont = default("<default>, 8px"); // Label font, automatic by default
        string labelColor = default("black"); // Label color, black by default

        string fadeOutMode @enum("realTime", "animationTime", "simulationTime") = default("realTime"); // Specifies how inactive links fade out
        double fadeOutTime @unit(s) = default(1s); // How quickly inactive links fade away, 1 second by default
        double fadeOutAnimationSpeed = default(0); // Animation speed while there are active links, value must be in the range (0, +inf)

        double holdAnimationTime @unit(s) = default(0s); // Hold simulation (no progress) for the specified amount of animation time when the link is activated

        @class(LinkVisualizerBase);
}

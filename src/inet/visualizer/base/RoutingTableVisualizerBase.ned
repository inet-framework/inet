//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for routing table visualizer simple modules. It visualizes
// routes from the IP routing tables of network nodes as arrows pointing from the
// source node to the gateway. This visualization helps understand how packets are
// forwarded through the network and how routing tables are configured.
//
// By default, routes are not displayed. To enable visualization, set the
// displayRoutingTables parameter to true and use the filters to select which
// routes to display.
//
// The visualizer dynamically updates when routing tables change by subscribing to
// the following signals: `routeAdded`, `routeChanged`, `routeDeleted`, and
// `interfaceIpv4ConfigChanged` on the modules determined by the
// `visualizationSubjectModule` parameter.
//
// The actual rendering of routes is implemented in derived modules for specific
// user interfaces (canvas or OSG).
//
// Label Format:
// The labelFormat parameter determines what information is displayed on the route
// arrows. It can contain the following directives:
//  - %m mask - Network mask
//  - %g gateway - Gateway address
//  - %d destination - Destination network address
//  - %e metric - Route metric
//  - %n interface name - Name of the outgoing interface
//  - %i IRoute::info() - Detailed route information
//  - %s IRoute::str() - String representation of the route
//
// @see ~RoutingTableCanvasVisualizer, ~RoutingTableOsgVisualizer, ~IRoutingTableVisualizer, ~VisualizerBase
//
simple RoutingTableVisualizerBase extends VisualizerBase
{
    parameters:
        // Visualization control
        bool displayRoutingTables = default(false); // Enables/disables routing table visualization, disabled by default
        bool displayRoutesIndividually = default(true); // When true, displays each route as a separate arrow even if they share source and destination nodes
        bool displayLabels = default(true); // Enables/disables labels on route arrows

        // Filtering parameters
        string destinationFilter @mutable = default("*"); // Selects which destination networks to visualize (supports wildcards and expressions)
        string nodeFilter @mutable = default("*"); // Selects which nodes' routing tables to visualize (supports wildcards and expressions)
        string multicastSourceNodeFilter @mutable = default("*"); // Selects which multicast source nodes to visualize
        string multicastSourceAddressFilter @mutable = default("*"); // Selects which multicast source addresses to visualize
        string multicastGroupFilter @mutable = default("*"); // Selects which multicast group addresses to visualize

        // Arrow appearance
        string lineColor = default("black"); // Color of the route arrows
        string lineStyle = default("solid"); // Style of the route arrows (solid, dashed, dotted)
        double lineWidth = default(2); // Width of the route arrows in pixels

        // Arrow positioning
        double lineShift = default(16); // Distance to shift arrows to avoid overlapping
        string lineShiftMode = default("normal"); // How to shift overlapping arrows: none, normal, x, y, z; optional prefix + or -
        double lineContactSpacing = default(3); // Gap between arrow end and node icon
        string lineContactMode @enum("circular", "rectangular") = default("rectangular"); // How arrows connect to node icons

        // Label appearance
        string labelFormat @mutable = default("%d/%m -> %g (%n)"); // Format string for route labels
        string labelFont = default("<default>, 8px"); // Font for route labels
        string labelColor = default("black"); // Color of route labels

        @class(RoutingTableVisualizerBase);
}

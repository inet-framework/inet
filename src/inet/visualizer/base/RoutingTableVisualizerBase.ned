//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for routing table visualizer simple modules. It displays
// routes in the routing tables of the network. Routes present are not displayed
// by default, see the module parameters for how to enable the visualization.
// The actual visualization of routes is done in derived modules.
//
// This module subscribes for routeAdded, routeChanged, routeDeleted,
// interfaceIpv4ConfigChanged on the module determined by the visualizationSubjectModule
// parameter.
//
// The format string can contain the following directives:
//  - %m mask
//  - %g gateway
//  - %d destination
//  - %e metric
//  - %n interface name
//  - %i IRoute::info()
//  - %s IRoute::str()
//
// @see ~RoutingTableCanvasVisualizer, ~RoutingTableOsgVisualizer, ~IRoutingTableVisualizer, ~VisualizerBase
//
simple RoutingTableVisualizerBase extends VisualizerBase
{
    parameters:
        bool displayRoutingTables = default(false); // Display routing table data, disabled by default
        bool displayRoutesIndividually = default(true); // Display routes between the same network nodes individually, enabled by default
        bool displayLabels = default(true); // Display labels on arrows, enabled by default

        string destinationFilter @mutable = default("*"); // Which destinations are considered, matches all destinations by default
        string nodeFilter @mutable = default("*"); // Which nodes are considered, matches all nodes by default
        string multicastSourceNodeFilter @mutable = default("*"); // Which multicast source nodes are considered, matches all nodes by default
        string multicastSourceAddressFilter @mutable = default("*"); // Which multicast source addresses are considered, matches all addresses by default
        string multicastGroupFilter @mutable = default("*"); // Which multicast group addresses are considered, matches all group addresses by default

        string lineColor = default("black"); // Route line color, black by default
        string lineStyle = default("solid"); // Route line style (solid, dashed, dotted)
        double lineWidth = default(2); // Route line width

        double lineShift = default(16); // Line shift to avoid overlapping lines
        string lineShiftMode = default("normal"); // Determines how overlapping lines are shifted, possible values are: none, normal, x, y, z; optional prefix + or -

        double lineContactSpacing = default(3); // Spacing between arrow end and submodule icon
        string lineContactMode @enum("circular", "rectangular") = default("rectangular"); // Determines how arrows are clipped at the submodules

        string labelFormat @mutable = default("%d/%m -> %g (%n)"); // Determines what route data is displayed
        string labelFont = default("<default>, 8px"); // Label font, automatic by default
        string labelColor = default("black"); // Label color, black by default

        @class(RoutingTableVisualizerBase);
}


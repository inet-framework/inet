//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for path visualizer simple modules. It keeps track of
// active routes in a network. A route between two node is considered active
// if a packet is sent recently between their corresponding layers. Active
// routes are not displayed by default, see the module parameters for how to
// enable the visualization. The actual visualization of active routes is done
// in derived modules.
//
// This module subscribes for packetReceivedFromUpper, packetSentToUpper, and
// packetReceivedFromLower on the module determined by the visualizationSubjectModule
// parameter. If the visualizer receives a packetReceivedFromUpper from a
// protocol module, then it stores the corresponding packet id. If later on the
// visualizer receives a packetSentToUpper from another corresponding protocol
// module with the same packet id, then it adds an active route between the
// source and the destination nodes. During the route discovery if the
// visualizer receives a packetReceivedFromLower from a corresponding protocol
// module, then it adds the node to the intermediate nodes of the route. After
// a certain amount of time the route becomes inactive unless it is reinforced
// by another packet.
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
        bool displayRoutes = default(false); // display polyline arrow for active routes, disabled by default

        string startPathSignal = default("packetReceivedFromUpper");
        string extendPathSignal = default("packetReceivedFromLower");
        string endPathSignal = default("packetSentToUpper");

        string nodeFilter @mutable = default("*"); // which nodes are considered, matches all nodes by default
        object packetFilter @mutable = default("*"); // which packets are considered, matches all packets by default

        string lineColor @enum("light", "dark") = default("dark"); // route color is a list of colors, a set of dark colors by default
        string lineStyle = default("solid"); // route line style (solid, dashed, dotted)
        double lineWidth = default(3); // route line width
        bool lineSmooth = default(false); // when true polylines are displayed as curves

        double lineShift = default(16); // route line shift to avoid overlapping routes
        string lineShiftMode = default("normal"); // determines how overlapping lines are shifted, possible values are: normal, x, y, z; optional prefix + or -

        double lineContactSpacing = default(3); // spacing between arrow end and submodule icon
        string lineContactMode @enum("circular", "rectangular") = default("rectangular"); // determines how arrows are clipped at the submodules

        string labelFormat = default("%n"); // determines what path data is displayed
        string labelFont = default("<default>, 8px"); // label font, automatic by default
        string labelColor = default(""); // label color, same as line by default

        string fadeOutMode @enum("realTime", "animationTime", "simulationTime") = default("realTime"); // specifies how inactive routes fade out
        double fadeOutTime @unit(s) = default(1s); // how quickly inactive routes fade away, 1 second by default
        double fadeOutAnimationSpeed = default(0); // animation speed while there are active routes, value must be in the range (0, +inf)

        @class(PathVisualizerBase);
}


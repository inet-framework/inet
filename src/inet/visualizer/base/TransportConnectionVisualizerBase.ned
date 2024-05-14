//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for transport connection visualizer simple modules. It
// keeps track of open transport connections in a network. Open connections are
// not displayed by default, see the module parameters for how to enable the
// visualization. The actual visualization of transport connections is done
// in derived modules.
//
// This module subscribes to tcpConnectionAdded and tcpConnectionRemoved on the
// module determined by the visualizationSubjectModule parameter.
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


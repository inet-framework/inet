//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for link break visualizer simple modules. It visualizes
// link failures in wireless and wired networks, showing where and when
// communication links between nodes have been broken.
//
// By default, link breaks are not displayed. To enable visualization, set the
// displayLinkBreaks parameter to true and use the filters to select which
// nodes, interfaces, and packets to monitor for link breaks.
//
// The visualizer shows link breaks as stop icons at the locations where
// links have failed. This helps identify connectivity issues, understand
// the impact of mobility on network topology, and debug routing protocols
// that need to adapt to changing network conditions.
//
// This module subscribes to `linkBroken` signals on the module determined by the
// `visualizationSubjectModule` parameter. When it receives a link break signal,
// it stores the broken link information and starts the visualization, which
// will fade out after the configured time.
//
// The actual rendering of link break indicators is implemented in derived
// modules for specific user interfaces (canvas or OSG).
//
// @see ~LinkBreakCanvasVisualizer, ~LinkBreakOsgVisualizer, ~ILinkBreakVisualizer, ~VisualizerBase
//
simple LinkBreakVisualizerBase extends VisualizerBase
{
    parameters:
        bool displayLinkBreaks = default(false); // Display icons for link breaks, disabled by default

        string nodeFilter @mutable = default("*"); // Which network nodes are considered, matches all nodes by default
        string interfaceFilter @mutable = default("*"); // Which interfaces are considered, matches all interfaces by default
        object packetFilter @mutable = default("*"); // Which packets are considered, matches all packets by default

        string icon = default("status/stop");
        double iconTintAmount = default(0);
        string iconTintColor = default("");

        string fadeOutMode @enum("realTime", "animationTime", "simulationTime") = default("realTime"); // Specifies how inactive link breaks fade out
        double fadeOutTime @unit(s) = default(1s); // How quickly link breaks fade away, 1 second by default
        double fadeOutAnimationSpeed = default(0); // Animation speed while there are link breaks, value must be in the range (0, +inf)

        @class(LinkBreakVisualizerBase);
}

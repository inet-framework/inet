//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for packet drop visualizer simple modules. It visualizes
// locations and reasons for packet drops in the network, helping identify
// communication problems and bottlenecks.
//
// By default, packet drops are not displayed. To enable visualization, set the
// displayPacketDrops parameter to true and use the filters to select which
// nodes, interfaces, packets, and drop reasons to visualize.
//
// The visualizer shows packet drops as icons at the locations where packets
// were discarded, with optional labels indicating the packet name or drop reason.
// Common drop reasons include queue overflow, address resolution failure,
// routing issues, TTL expiration, and bit errors.
//
// This module subscribes to `packetDropped` signals on the module determined by the
// `visualizationSubjectModule` parameter. When it receives a packet dropped
// signal, it stores the dropped packet information and starts the visualization,
// which will fade out after the configured time.
//
// The format string can contain the following directives:
//  - %n packet name
//  - %c packet class
//  - %r drop reason number
//  - %s drop reason string
//
// @see ~PacketDropCanvasVisualizer, ~PacketDropOsgVisualizer, ~IPacketDropVisualizer, ~VisualizerBase
//
simple PacketDropVisualizerBase extends VisualizerBase
{
    parameters:
        bool displayPacketDrops = default(false); // Display packet drops, disabled by default

        string nodeFilter @mutable = default("*"); // Which network nodes are considered, matches all network nodes by default
        string interfaceFilter @mutable = default("*"); // Which network interfaces are considered, matches all network interfaces by default
        object packetFilter @mutable = default("*"); // Which packets are considered, matches all packets by default
        string detailsFilter @mutable = default("*"); // Which packet drops are considered, matches all packet drops by default

        string icon = default("msg/packet_s"); // Packet drop visualization icon
        double iconTintAmount = default(1); // Colorization amount between 0 and 1, enabled by default
        string iconTintColor = default("dark"); // Icon tint color is a list of colors, a set of dark colors by default

        string labelFormat @mutable = default("%n"); // Determines what data is displayed in the label, packet name by default
        string labelFont = default("<default>"); // Label font, automatic by default
        string labelColor = default("black"); // Label color, black by default

        string fadeOutMode @enum("realTime", "animationTime", "simulationTime") = default("realTime"); // Specifies how inactive packet drops fade out, real time by default
        double fadeOutTime @unit(s) = default(1s); // How quickly packet drops fade away, 1 second by default
        double fadeOutAnimationSpeed = default(0); // Animation speed while there are packet drops, value must be in the range (0, +inf), disabled by default

        @class(PacketDropVisualizerBase);
}

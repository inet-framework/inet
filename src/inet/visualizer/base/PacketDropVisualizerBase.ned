//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for packet drop visualizer simple modules. It keeps
// track of dropped packets in a network. Packet drops are not displayed by
// default, see module parameters for how to enable the visualization. The
// actual visualization of packet drops is done in derived modules.
//
// This module subscribes to `packetDropped` on the module determined by the
// `visualizationSubjectModule` parameter. If the visualizer receives a packet dropped
// signal, then it stores the dropped packet and starts the visualization.
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


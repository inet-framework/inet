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
// This module subscribes for packetDropped on the module determined by the
// visualizationSubjectModule parameter. If the visualizer receives a packet dropped
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
        bool displayPacketDrops = default(false); // display packet drops, disabled by default

        string nodeFilter @mutable = default("*"); // which network nodes are considered, matches all network nodes by default
        string interfaceFilter @mutable = default("*"); // which network interfaces are considered, matches all network interfaces by default
        object packetFilter @mutable = default("*"); // which packets are considered, matches all packets by default
        string detailsFilter @mutable = default("*"); // which packet drops are considered, matches all packet drops by default

        string icon = default("msg/packet_s"); // packet drop visualization icon
        double iconTintAmount = default(1); // colorization amount between 0 and 1, enabled by default
        string iconTintColor = default("dark"); // icon tint color is a list of colors, a set of dark colors by default

        string labelFormat @mutable = default("%n"); // determines what data is displayed in the label, packet name by default
        string labelFont = default("<default>"); // label font, automatic by default
        string labelColor = default("black"); // label color, black by default

        string fadeOutMode @enum("realTime", "animationTime", "simulationTime") = default("realTime"); // specifies how inactive packet drops fade out, real time by default
        double fadeOutTime @unit(s) = default(1s); // how quickly packet drops fade away, 1 second by default
        double fadeOutAnimationSpeed = default(0); // animation speed while there are packet drops, value must be in the range (0, +inf), disabled by default

        @class(PacketDropVisualizerBase);
}


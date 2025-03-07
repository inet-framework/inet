//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for channel visualizer simple modules. It visualizes
// communication channels as arrows pointing from source to destination nodes.
// This visualization helps understand active communication paths and traffic
// patterns in the network.
//
// By default, channels are not displayed. To enable visualization, set the
// displayChannelActivity parameter to true and use the filters to select which
// channels to display.
//
// The visualizer dynamically updates when packets are transmitted through
// channels, showing active communication paths and fading them out when they
// become inactive.
//
// The actual rendering of channels is implemented in derived modules for specific
// user interfaces (canvas or OSG).
//
// Label Format:
// The labelFormat parameter determines what information is displayed on the channel
// arrows. It can contain the following directives:
//  - %n name - Channel name
//
// @see ~ChannelCanvasVisualizer, ~ChannelOsgVisualizer, ~ChannelVisualizer, ~IChannelVisualizer
//
simple ChannelVisualizerBase extends VisualizerBase
{
    parameters:
        bool displayChannelActivity = default(false); // Display arrows for active channels. Disabled by default.

        string nodeFilter @mutable = default("*"); // Which nodes are considered. Matches all nodes by default.
        object packetFilter @mutable = default("*"); // Which packets are considered. Matches all packets by default.

        string lineColor = default("blue"); // Arrow color. Black by default.
        string lineStyle = default("solid"); // Arrow style (solid, dashed, dotted).
        double lineWidth = default(3); // Arrow width.

        double lineShift = default(16); // Line shift to avoid overlapping arrows.
        string lineShiftMode = default("normal"); // Determines how overlapping arrows are shifted. Possible values are: none, normal, x, y, z; optional prefix + or -.

        double lineContactSpacing = default(3); // Spacing between arrow end and submodule icon.
        string lineContactMode @enum("circular", "rectangular") = default("rectangular"); // Determines how arrows are clipped at the submodules.

        string labelFormat = default("%n"); // Determines what data is displayed.
        string labelFont = default("<default>, 8px"); // Label font. Automatic by default.
        string labelColor = default("blue"); // Label color. Black by default.

        string fadeOutMode @enum("realTime", "animationTime", "simulationTime") = default("realTime"); // Specifies how inactive channels fade out.
        double fadeOutTime @unit(s) = default(1s); // How quickly inactive channels fade away. 1 second by default.
        double fadeOutAnimationSpeed = default(0); // Animation speed while there are active channels. Value must be in the range (0, +inf).

        double holdAnimationTime @unit(s) = default(0s); // Hold simulation (no progress) for the specified amount of animation time when the channel is activated.

        @class(ChannelVisualizerBase);
}

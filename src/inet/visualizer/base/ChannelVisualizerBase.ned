//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

simple ChannelVisualizerBase extends VisualizerBase
{
    parameters:
        bool displayChannelActivity = default(false); // Display arrows for active channels, disabled by default

        string nodeFilter @mutable = default("*"); // Which nodes are considered, matches all nodes by default
        object packetFilter @mutable = default("*"); // Which packets are considered, matches all packets by default

        string lineColor = default("blue"); //  arrow color, black by default
        string lineStyle = default("solid"); // Arrow style (solid, dashed, dotted)
        double lineWidth = default(3); // Arrow width

        double lineShift = default(16); // Line shift to avoid overlapping arrows
        string lineShiftMode = default("normal"); // Determines how overlapping arrows are shifted, possible values are: none, normal, x, y, z; optional prefix + or -

        double lineContactSpacing = default(3); // Spacing between arrow end and submodule icon
        string lineContactMode @enum("circular", "rectangular") = default("rectangular"); // Determines how arrows are clipped at the submodules

        string labelFormat = default("%n"); // Determines what data is displayed
        string labelFont = default("<default>, 8px"); // Label font, automatic by default
        string labelColor = default("blue"); // Label color, black by default

        string fadeOutMode @enum("realTime", "animationTime", "simulationTime") = default("realTime"); // Specifies how inactive channels fade out
        double fadeOutTime @unit(s) = default(1s); // How quickly inactive channels fade away, 1 second by default
        double fadeOutAnimationSpeed = default(0); // Animation speed while there are active channels, value must be in the range (0, +inf)

        double holdAnimationTime @unit(s) = default(0s); // Hold simulation (no progress) for the specified amount of animation time when the channel is activated

        @class(ChannelVisualizerBase);
}


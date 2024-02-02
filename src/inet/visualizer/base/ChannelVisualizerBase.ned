//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

simple ChannelVisualizerBase extends VisualizerBase
{
    parameters:
        bool displayChannelActivity = default(false); // display arrows for active channels, disabled by default

        string nodeFilter @mutable = default("*"); // which nodes are considered, matches all nodes by default
        object packetFilter @mutable = default("*"); // which packets are considered, matches all packets by default

        string lineColor = default("blue"); //  arrow color, black by default
        string lineStyle = default("solid"); // arrow style (solid, dashed, dotted)
        double lineWidth = default(3); // arrow width

        double lineShift = default(16); // line shift to avoid overlapping arrows
        string lineShiftMode = default("normal"); // determines how overlapping arrows are shifted, possible values are: none, normal, x, y, z; optional prefix + or -

        double lineContactSpacing = default(3); // spacing between arrow end and submodule icon
        string lineContactMode @enum("circular", "rectangular") = default("rectangular"); // determines how arrows are clipped at the submodules

        string labelFormat = default("%n"); // determines what data is displayed
        string labelFont = default("<default>, 8px"); // label font, automatic by default
        string labelColor = default("blue"); // label color, black by default

        string fadeOutMode @enum("realTime", "animationTime", "simulationTime") = default("realTime"); // specifies how inactive channels fade out
        double fadeOutTime @unit(s) = default(1s); // how quickly inactive channels fade away, 1 second by default
        double fadeOutAnimationSpeed = default(0); // animation speed while there are active channels, value must be in the range (0, +inf)

        double holdAnimationTime @unit(s) = default(0s); // hold simulation (no progress) for the specified amount of animation time when the channel is activated

        @class(ChannelVisualizerBase);
}


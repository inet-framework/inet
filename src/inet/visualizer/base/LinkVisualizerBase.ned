//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for link visualizer simple modules. It keeps track of
// active links in a network. A link between two network node is considered
// active if a packet is sent recently between the corresponding layers. Active
// links are not displayed by default, see the module parameters for how to
// enable the visualization. The actual visualization of active links is done
// in derived modules.
//
// This module subscribes for signals at the provided subscription module. When
// determining active links, it only considers packets matched by the provided
// filters. The filters use the format of cMatchExpression with the packet name
// as the default attribute.
//
// The format string can contain the following directives:
//  - %n packet name
//  - %c packet class
//
// @see ~PhysicalLinkVisualizerBase, ~DataLinkVisualizerBase, ~VisualizerBase
//
simple LinkVisualizerBase extends VisualizerBase
{
    parameters:
        bool displayLinks = default(false); // display arrows for active links, disabled by default
        string activityLevel @enum("service", "peer", "protocol") = default("service"); // determines what level of activity is displayed, service level by default

        string nodeFilter @mutable = default("*"); // which nodes are considered, matches all nodes by default
        string interfaceFilter @mutable = default("*"); // which interfaces are considered, matches all interfaces by default
        object packetFilter @mutable = default("*"); // which packets are considered, matches all packets by default

        string lineColor = default("black"); // link arrow color, black by default
        string lineStyle = default("solid"); // link arrow style (solid, dashed, dotted)
        double lineWidth = default(3); // link arrow width

        double lineShift = default(16); // line shift to avoid overlapping arrows
        string lineShiftMode = default("normal"); // determines how overlapping arrows are shifted, possible values are: none, normal, x, y, z; optional prefix + or -

        double lineContactSpacing = default(3); // spacing between arrow end and submodule icon
        string lineContactMode @enum("circular", "rectangular") = default("rectangular"); // determines how arrows are clipped at the submodules

        string labelFormat = default("%n"); // determines what link data is displayed
        string labelFont = default("<default>, 8px"); // label font, automatic by default
        string labelColor = default("black"); // label color, black by default

        string fadeOutMode @enum("realTime", "animationTime", "simulationTime") = default("realTime"); // specifies how inactive links fade out
        double fadeOutTime @unit(s) = default(1s); // how quickly inactive links fade away, 1 second by default
        double fadeOutAnimationSpeed = default(0); // animation speed while there are active links, value must be in the range (0, +inf)

        double holdAnimationTime @unit(s) = default(0s); // hold simulation (no progress) for the specified amount of animation time when the link is activated

        @class(LinkVisualizerBase);
}


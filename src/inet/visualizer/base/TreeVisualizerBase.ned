//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

// TODO what?
// This is a base module for tree visualizer simple modules. It visualizes
// tree structures in the network, such as multicast trees, spanning trees,
// or other hierarchical relationships between network nodes.
//
// By default, trees are not displayed. To enable visualization, set the
// displayTrees parameter to true and configure the derived visualizer module
// with appropriate parameters for the specific tree type.
//
// The visualizer shows tree structures as a set of connected arrows between
// network nodes, with the root node at the top and child nodes below. This
// helps understand hierarchical relationships, routing paths, and network
// organization.
//
// The actual rendering of tree structures is implemented in derived modules
// for specific tree types and user interfaces (canvas or OSG).
//
// @see ~VisualizerBase
//
simple TreeVisualizerBase extends VisualizerBase
{
    parameters:
        bool displayTrees = default(false); // Display a set of polyline arrows for trees, disabled by default

        string lineColor = default("dark"); // Line color is a list of colors or a color group name (e.g. dark, light), a set of dark colors by default
        string lineStyle = default("solid"); // Line style (solid, dashed, dotted)
        double lineWidth = default(3); // Line width
        bool lineSmooth = default(false); // When true polylines are displayed as curves

        double lineShift = default(16); // Line shift to avoid overlapping lines
        string lineShiftMode = default("normal"); // Determines how overlapping lines are shifted, possible values are: normal, x, y, z; optional prefix + or -

        double lineContactSpacing = default(3); // Spacing between arrow end and submodule icon
        string lineContactMode @enum("circular", "rectangular") = default("rectangular"); // Determines how arrows are clipped at the submodules

        @class(TreeVisualizerBase);
}

//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

simple TreeVisualizerBase extends VisualizerBase
{
    parameters:
        bool displayTrees = default(false); // display a set of polyline arrows for trees, disabled by default

        string lineColor @enum("light", "dark") = default("dark"); // line color is a list of colors, a set of dark colors by default
        string lineStyle = default("solid"); // line style (solid, dashed, dotted)
        double lineWidth = default(3); // line width
        bool lineSmooth = default(false); // when true polylines are displayed as curves

        double lineShift = default(16); // line shift to avoid overlapping lines
        string lineShiftMode = default("normal"); // determines how overlapping lines are shifted, possible values are: normal, x, y, z; optional prefix + or -

        double lineContactSpacing = default(3); // spacing between arrow end and submodule icon
        string lineContactMode @enum("circular", "rectangular") = default("rectangular"); // determines how arrows are clipped at the submodules

        @class(TreeVisualizerBase);
}


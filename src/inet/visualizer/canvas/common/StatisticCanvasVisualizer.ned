//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.common;

import inet.visualizer.base.StatisticVisualizerBase;
import inet.visualizer.contract.IStatisticVisualizer;

//
// This module visualizes statistics on a 2D canvas. See the base module for
// how to configure which statistic is displayed. If configured properly, then
// it displays the last value of the statistic to the right of the visualization
// of the corresponding network node. The value and the unit is wrapped in a
// filled white rectangle.
//
// The statistic value is display with a label by default. Alternatively, any
// indicator figure can be used by configuring the property parameters and
// providing a figure template along the module path of the statistic visualizer.
//
// @see ~StatisticOsgVisualizer, ~StatisticVisualizer, ~StatisticVisualizerBase, ~IStatisticVisualizer
//
simple StatisticCanvasVisualizer extends StatisticVisualizerBase like IStatisticVisualizer
{
    parameters:
        double zIndex = default(10); // determines the drawing order of figures relative to other visualizers
        string propertyName = default(""); // optional property name of a figure template along the module path of the visualizer
        string propertyIndex = default(""); // optional property index of a figure template along the module path of the visualizer
        @class(StatisticCanvasVisualizer);
}


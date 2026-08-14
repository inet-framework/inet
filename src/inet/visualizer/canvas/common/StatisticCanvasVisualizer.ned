//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.common;

import inet.visualizer.base.StatisticVisualizerBase;
import inet.visualizer.contract.IStatisticVisualizer;

//
// Visualizes statistics on a 2D canvas. See the base module for
// how to configure which statistic is displayed. If configured properly, then
// it displays the last value of the statistic to the right of the visualization
// of the corresponding network node. The value and the unit are wrapped in a
// filled white rectangle.
//
// The statistic value is displayed with a label by default. Alternatively, any
// indicator figure can display it, either configured in the `figure` parameter,
// or by providing a figure template along the module path of the statistic
// visualizer and referring to it with the property parameters. Displaying
// several values at once, as configured by the `splitBy` and `groupBy` parameters
// of the base module, needs a figure that displays several items, e.g. a bar chart.
//
// @see ~StatisticOsgVisualizer, ~StatisticVisualizer, ~StatisticVisualizerBase, ~IStatisticVisualizer
//
simple StatisticCanvasVisualizer extends StatisticVisualizerBase like IStatisticVisualizer
{
    parameters:
        double zIndex = default(10); // Determines the drawing order of figures relative to other visualizers

        // The figure that displays the value(s), given as the attributes of a figure the
        // same way as in an @figure property, e.g. {type: "gauge", minValue: 0, maxValue:
        // 100, tickSize: 20}. The type selects the figure class registered with
        // Register_Figure(), the other attributes are the parameters of that particular
        // figure. Empty means a text label, or a bar chart ({type: "barChart"}) if
        // `groupBy` is not "none".
        object figure = default({});

        string propertyName = default(""); // Optional property name of a figure template along the module path of the visualizer, an alternative to the figure parameter, taking precedence over it
        string propertyIndex = default(""); // Optional property index of a figure template along the module path of the visualizer
        @class(StatisticCanvasVisualizer);
}


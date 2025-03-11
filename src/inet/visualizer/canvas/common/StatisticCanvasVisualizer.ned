//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.common;

import inet.visualizer.base.StatisticVisualizerBase;
import inet.visualizer.contract.IStatisticVisualizer;

//
// Visualizes statistics on a 2D canvas. Displays the last values of a selected
// statistic collected during simulation.
//
// The visualization includes:
// - Text labels showing statistic values next to network nodes
// - Values and units wrapped in filled rectangles for better visibility
// - Automatic updates as statistic values change during simulation TODO trivial
// - Support for custom indicator figures through templates TODO?
// - Filtering options to visualize only specific statistics or nodes
// - Options to filter for specific statistics or nodes TODO v2
//
// Particularly useful for monitoring performance metrics, analyzing protocol
// behavior, comparing different configurations, and creating informative
// visualizations for presentations or debugging.
//
// @see ~StatisticOsgVisualizer, ~StatisticVisualizer, ~StatisticVisualizerBase, ~IStatisticVisualizer
//
simple StatisticCanvasVisualizer extends StatisticVisualizerBase like IStatisticVisualizer
{
    parameters:
        double zIndex = default(10); // Determines the drawing order of figures relative to other visualizers
        string propertyName = default(""); // Optional property name of a figure template along the module path of the visualizer
        string propertyIndex = default(""); // Optional property index of a figure template along the module path of the visualizer
        @class(StatisticCanvasVisualizer);
}

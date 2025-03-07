//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for statistic visualizer simple modules. It visualizes
// various statistics from network nodes as text annotations, allowing real-time
// monitoring of simulation metrics.
//
// By default, statistics are not displayed. To enable visualization, set the
// displayStatistics parameter to true, specify the signalName and statisticName
// parameters, and use the sourceFilter to select which modules to monitor.
//
// The visualizer tracks the last value of a specified statistic for multiple
// network nodes. It subscribes to the signal determined by the signalName
// parameter at the module specified by the visualizationSubjectModule parameter.
// When it receives such a signal, it checks if the source matches the sourceFilter
// parameter. If the signal passes the check, it determines and displays the last
// value of the statistic specified by the statisticName parameter.
//
// This visualizer is particularly useful for monitoring performance metrics,
// analyzing protocol behavior, debugging simulation issues, and observing
// the dynamic evolution of network parameters during simulation.
//
// The format string can contain the following directives:
//  - %s: signal name
//  - %n: statistic name
//  - %v: statistic value
//  - %u: statistic unit
//
// @see ~StatisticCanvasVisualizer, ~StatisticOsgVisualizer, ~IStatisticVisualizer, ~VisualizerBase
//
simple StatisticVisualizerBase extends VisualizerBase
{
    parameters:
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // The network node visualizer module, refers to a sibling submodule by default

        bool displayStatistics = default(false);

        string signalName = default(""); // Signal name to subscribe, no signal name by default
        string statisticName = default(signalName); // Statistic name to get the value, signal name by default
        string statisticUnit = default(""); // Statistic unit, undefined by default
        string statisticExpression = default(""); // Statistic expression, received value by default
        string sourceFilter @mutable = default("**"); // Pattern that must match the signal source, no source filter by default

        string format @mutable = default("%n: %v %u"); // Determines what is displayed, info by default
        string unit = default(""); // Display the value in one of the specified units, statistic unit by default

        string font = default("<default>"); // Statistic value font, automatic by default
        string textColor = default("black"); // Statistic value font color, black by default
        string backgroundColor = default("white"); // Statistic value background color, white by default
        double opacity = default(1.0); // Opacity of the visualization
        string placementHint = default("top"); // Annotation placement hint, space separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double placementPriority = default(0); // Determines the order of annotation positioning

        @class(StatisticVisualizerBase);
}

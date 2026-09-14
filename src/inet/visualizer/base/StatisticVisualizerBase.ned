//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// Base module for statistic visualizer simple modules. It keeps track
// of the last value of a statistic for multiple network nodes. The actual
// visualization of statistic values is done in derived modules. It subscribes
// for the signal determined by the `signalName` parameter at the module specified
// by the `visualizationSubjectModule` parameter. If the visualizer receives such a signal,
// it checks if the source matches the `sourceFilter` parameter. If the signal
// passes the check, it determines the last value of the statistic specified by
// the `statisticName` parameter.
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

        // Determines whether the values a signal source emits are split into several
        // statistics, and what identifies each of them:
        //  - "none": the source emits the values of a single statistic
        //  - "details": one statistic per distinct details object emitted with the value,
        //    i.e. the live counterpart of the demux() result filter
        //  - "flow": one statistic per packet flow of the source, in which case
        //    `statisticExpression` must contain demuxFlow()
        // Splitting requires `groupBy` = "source", because the statistics of one source are
        // what is displayed together.
        string splitBy @enum("none","details","flow") = default("none");

        // Determines which statistics are displayed together as the items of a single
        // figure, which then needs to be one that displays several items, such as a bar
        // chart:
        //  - "none": each statistic is displayed on a figure of its own
        //  - "source": the statistics of one signal source are displayed together, i.e. the
        //    ones `splitBy` split its values into; requires a `splitBy` other than "none",
        //    because without splitting a source has a single statistic
        //  - "networkNode": the matching signal sources of one network node are displayed
        //    together, one item per source; requires `splitBy` = "none"
        // With `groupBy` = "networkNode" and with `splitBy` = "flow" the values come from the
        // result recorders built from `statisticExpression`, so an item can display e.g. a
        // count or a throughput rather than the value of the signal.
        string groupBy @enum("none","source","networkNode") = default("none");

        @class(StatisticVisualizerBase);
}


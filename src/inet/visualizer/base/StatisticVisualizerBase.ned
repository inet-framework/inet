//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for statistic visualizer simple modules. It keeps track
// of the last value of a statistic for multiple network nodes. The actual
// visualization of statistic values is done in derived modules. It subscribes
// for the signal determined by the signalName parameter at the module specified
// by the visualizationSubjectModule parameter. If the visualizer receives such a signal
// it checks if the source matches the sourceFilter parameter. If the signal
// passes the check it determines the last value of the statistic specified by
// the statisticName parameter.
//
// The format string can contain the following directives:
//  - %s signal name
//  - %n statistic name
//  - %v statistic value
//  - %u statistic unit
//
// @see ~StatisticCanvasVisualizer, ~StatisticOsgVisualizer, ~IStatisticVisualizer, ~VisualizerBase
//
simple StatisticVisualizerBase extends VisualizerBase
{
    parameters:
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // the network node visualizer module, refers to a sibling submodule by default

        bool displayStatistics = default(true);

        string signalName = default(""); // signal name to subscribe, no signal name by default
        string statisticName = default(signalName); // statistic name to get the value, signal name by default
        string statisticUnit = default(""); // statistic unit, undefined by default
        string statisticExpression = default(""); // statistic expression, received value by default
        string sourceFilter @mutable = default("**"); // pattern that must match the signal source, no source filter by default

        string format @mutable = default("%n: %v %u"); // determines what is displayed, info by default
        string unit = default(""); // display the value in one of the specified units, statistic unit by default

        string font = default("<default>"); // statistic value font, automatic by default
        string textColor = default("black"); // statistic value font color, black by default
        string backgroundColor = default("white"); // statistic value background color, white by default
        double opacity = default(1.0); // opacity of the visualization
        string placementHint = default("top"); // annotation placement hint, space separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double placementPriority = default(0); // determines the order of annotation positioning

        @class(StatisticVisualizerBase);
}


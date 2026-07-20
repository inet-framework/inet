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

        // Bar chart mode: instead of a value label per source, display a small bar chart per source, with
        // one bar per distinct demux label (the full name of the details object emitted with the value).
        // This is the live counterpart of the demux() result filter used for recording.
        string displayMode @enum("text","bars") = default("text"); // "text": a value label per source (default); "bars": a bar chart per source
        string barSeries @enum("details","sources","flow") = default("details"); // Bar keying in bars mode: "details" = one bar per demux label of a single source's signal; "sources" = one bar per matching source module, grouped by network node; "flow" = one bar per flow, demultiplexing a single source's signal by the packet's flow tag (the statisticExpression must contain demuxFlow). "sources"/"flow" take the bar value from statisticExpression, e.g. count or throughput
        double maxValue = default(0); // Value mapped to a full-height bar and to the top of the color gradient (in the display unit)
        double minValue = default(0); // Value mapped to a zero-height bar and to the bottom of the color gradient (in the display unit)
        double barWidth = default(12); // Width of each bar in pixels
        double barSpacing = default(8); // Horizontal spacing between adjacent bars in pixels
        double maxBarHeight = default(60); // Height of a full bar in pixels
        string barColor = default("blue"); // A single bar color, or a value-to-color gradient (list of colors from minValue to maxValue), or a color group name
        string valueFormat = default("%g"); // printf format for the value label on top of each bar (value in the display unit)
        string valueLabelFont = default("<default>,8"); // Value label font
        string valueLabelColor = default("black"); // Value label color
        string seriesLabelFont = default("<default>,8"); // Per-bar series (demux label) font
        string seriesLabelColor = default("black"); // Per-bar series label color
        double nameRotation @unit(deg) = default(45deg); // Rotation angle of the per-bar series labels
        bool displayTitle = default(true); // Display a title above each bar chart
        string titleFont = default("<default>,9"); // Chart title font
        string titleColor = default("black"); // Chart title color

        @class(StatisticVisualizerBase);
}


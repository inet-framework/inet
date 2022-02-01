//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.common;

import inet.visualizer.base.StatisticVisualizerBase;
import inet.visualizer.contract.IStatisticVisualizer;

//
// This module visualizes a statistic on a 3D osg scene. See the base module for
// how to configure which statistic is displayed. If configured properly, then
// it displays the last value of the statistic just above the visualization of
// the corresponding network node. The value and the unit is wrapped in a filled
// rectangle.
//
// @see ~StatisticCanvasVisualizer, ~StatisticVisualizer, ~StatisticVisualizerBase, ~IStatisticVisualizer
//
simple StatisticOsgVisualizer extends StatisticVisualizerBase like IStatisticVisualizer
{
    parameters:
        @class(StatisticOsgVisualizer);
}


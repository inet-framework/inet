//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.vsg.common;

import inet.visualizer.base.StatisticVisualizerBase;
import inet.visualizer.contract.IStatisticVisualizer;

//
// Visualizes a statistic on a 3D VulkanSceneGraph scene. See the base module for
// how to configure which statistic is displayed. If configured properly, then
// it displays the last value of the statistic just above the visualization of
// the corresponding network node. The value and the unit are wrapped in a label.
//
// @see ~StatisticCanvasVisualizer, ~StatisticVisualizer, ~StatisticVisualizerBase, ~IStatisticVisualizer
//
simple StatisticVsgVisualizer extends StatisticVisualizerBase like IStatisticVisualizer
{
    parameters:
        @class(StatisticVsgVisualizer);
}

//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.common;

import inet.visualizer.base.StatisticVisualizerBase;
import inet.visualizer.contract.IStatisticVisualizer;

//
// Visualizes statistics in a 3D scene using OpenSceneGraph (OSG). Displays
// numerical values of various metrics collected during simulation.
//
// The visualization includes:
// - Text labels showing statistic values above network nodes
// - Values and units wrapped in filled rectangles for better visibility TODO?
// - Automatic updates as statistic values change during simulation
// - Support for custom indicator figures through templates
// - Filtering options to visualize only specific statistics or nodes
//
// Particularly useful for monitoring performance metrics in 3D view, analyzing
// protocol behavior, comparing different configurations, and creating informative
// visualizations for presentations or debugging in 3D view.
//
// @see ~StatisticCanvasVisualizer, ~StatisticVisualizer, ~StatisticVisualizerBase, ~IStatisticVisualizer
//
simple StatisticOsgVisualizer extends StatisticVisualizerBase like IStatisticVisualizer
{
    parameters:
        @class(StatisticOsgVisualizer);
}

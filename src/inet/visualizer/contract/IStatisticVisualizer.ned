//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for statistic visualizers. Statistic visualizers display numerical
// values and metrics from simulation modules as text or graphical indicators.
//
// @see ~StatisticCanvasVisualizer, ~StatisticOsgVisualizer, ~StatisticVisualizer, ~StatisticVisualizerBase
//
moduleinterface IStatisticVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}

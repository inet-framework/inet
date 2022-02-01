//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// This module interface allows configuring different ~IStatisticVisualizer
// submodules from INI files. Such a visualizer displays a statistic for
// multiple network nodes.
//
// @see ~StatisticCanvasVisualizer, ~StatisticOsgVisualizer, ~StatisticVisualizer, ~StatisticVisualizerBase
//
moduleinterface IStatisticVisualizer
{
    parameters:
        @display("i=block/app");
}


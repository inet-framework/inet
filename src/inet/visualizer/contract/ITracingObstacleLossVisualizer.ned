//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// This module interface allows configuring different
// ~ITracingObstacleLossVisualizer submodules from INI files. Such a visualizer
// displays obstacle loss during wireless signal propagation.
//
// @see ~TracingObstacleLossCanvasVisualizer, ~TracingObstacleLossOsgVisualizer, ~TracingObstacleLossVisualizer, ~TracingObstacleLossVisualizerBase
//
moduleinterface ITracingObstacleLossVisualizer
{
    parameters:
        @display("i=block/app");
}


//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for tracing obstacle loss visualizers. These visualizers display
// how wireless signals interact with physical obstacles, showing signal
// reflections, refractions, and attenuations.
//
// @see ~TracingObstacleLossCanvasVisualizer, ~TracingObstacleLossOsgVisualizer, ~TracingObstacleLossVisualizer, ~TracingObstacleLossVisualizerBase
//
moduleinterface ITracingObstacleLossVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}

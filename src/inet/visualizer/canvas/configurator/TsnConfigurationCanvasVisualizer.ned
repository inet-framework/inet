//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.configurator;

import inet.visualizer.base.TreeVisualizerBase;
import inet.visualizer.contract.IFailureProtectionConfigurationVisualizer;

simple FailureProtectionConfigurationCanvasVisualizer extends TreeVisualizerBase like IFailureProtectionConfigurationVisualizer
{
    parameters:
        string failureProtectionConfiguratorModule = default("failureProtectionConfigurator");
        string streamFilter = default("*");
        double zIndex = default(3); // determines the drawing order of figures relative to other visualizers
        @class(FailureProtectionConfigurationCanvasVisualizer);
}


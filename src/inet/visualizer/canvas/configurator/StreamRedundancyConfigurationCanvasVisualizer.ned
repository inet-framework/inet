//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.configurator;

import inet.visualizer.base.TreeVisualizerBase;
import inet.visualizer.contract.IStreamRedundancyConfigurationVisualizer;

simple StreamRedundancyConfigurationCanvasVisualizer extends TreeVisualizerBase like IStreamRedundancyConfigurationVisualizer
{
    parameters:
        string streamRedundancyConfiguratorModule = default("streamRedundancyConfigurator");
        string streamFilter = default("*");
        double zIndex = default(3); // determines the drawing order of figures relative to other visualizers
        @class(StreamRedundancyConfigurationCanvasVisualizer);
}


//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.configurator;

import inet.visualizer.base.TreeVisualizerBase;
import inet.visualizer.contract.IStreamRedundancyConfigurationVisualizer;

//
// Visualizes stream redundancy configurations on a canvas by displaying
// polyline arrows representing redundant paths for streams. Uses information
// from the ~StreamRedundancyConfigurator module to identify and display the
// network paths used for redundant stream transmission in Time-Sensitive
// Networking (TSN) applications.
//
// @see StreamRedundancyConfigurator
//
simple StreamRedundancyConfigurationCanvasVisualizer extends TreeVisualizerBase like IStreamRedundancyConfigurationVisualizer
{
    parameters:
        string streamRedundancyConfiguratorModule = default("streamRedundancyConfigurator");
        string streamFilter = default("*");
        double zIndex = default(3); // Determines the drawing order of figures relative to other visualizers
        @class(StreamRedundancyConfigurationCanvasVisualizer);
}


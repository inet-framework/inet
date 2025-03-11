//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.configurator;

import inet.visualizer.base.TreeVisualizerBase;
import inet.visualizer.contract.IStreamRedundancyConfigurationVisualizer;

// TODO
// Visualizes stream redundancy configurations on a 2D canvas. Displays redundant
// paths for data streams in Time-Sensitive Networking (TSN) applications.
//
// The visualization includes:
// - Tree-like structures showing multiple redundant paths for streams
// - Colored lines representing different stream paths
// - Customizable line styles and appearance
// - Filtering options to visualize only specific streams
//
// Particularly useful for verifying TSN redundancy configurations, analyzing
// stream replication and elimination paths, debugging redundancy mechanisms,
// and understanding how the network provides reliability through redundancy.
//
// @see ~StreamRedundancyConfigurationOsgVisualizer, ~StreamRedundancyConfigurationVisualizer, ~TreeVisualizerBase, ~IStreamRedundancyConfigurationVisualizer
//
simple StreamRedundancyConfigurationCanvasVisualizer extends TreeVisualizerBase like IStreamRedundancyConfigurationVisualizer
{
    parameters:
        string streamRedundancyConfiguratorModule = default("streamRedundancyConfigurator");
        string streamFilter = default("*");
        double zIndex = default(3); // Determines the drawing order of figures relative to other visualizers
        @class(StreamRedundancyConfigurationCanvasVisualizer);
}

//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.configurator;

import inet.visualizer.base.TreeVisualizerBase;
import inet.visualizer.contract.IFailureProtectionConfigurationVisualizer;

//
// Visualizes failure protection configurations on a 2D canvas. Displays the
// redundant paths configured for network resilience.
//
// The visualization includes:
// - Tree-like structures showing redundant paths through the network
// - Colored lines representing different protection paths
// - Customizable line styles and appearance
// - Filtering options to visualize only specific streams
//
// Particularly useful for verifying network resilience designs, analyzing protection
// path configurations, debugging redundancy mechanisms TODO?, and understanding how the
// network will respond to failures.
//
// @see ~FailureProtectionConfigurationOsgVisualizer, ~FailureProtectionConfigurationVisualizer, ~TreeVisualizerBase, ~IFailureProtectionConfigurationVisualizer
//
simple FailureProtectionConfigurationCanvasVisualizer extends TreeVisualizerBase like IFailureProtectionConfigurationVisualizer
{
    parameters:
        string failureProtectionConfiguratorModule = default("failureProtectionConfigurator");
        string streamFilter = default("*");
        double zIndex = default(3); // Determines the drawing order of figures relative to other visualizers
        @class(FailureProtectionConfigurationCanvasVisualizer);
}

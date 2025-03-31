//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.visualizer.canvas.configurator;

import inet.visualizer.base.TreeVisualizerBase;
import inet.visualizer.contract.IFailureProtectionConfigurationVisualizer;

//
// Visualizes failure protection configurations on a canvas by displaying
// polyline arrows representing redundant paths for streams. Uses information
// from the FailureProtectionConfigurator module to identify and display the
// network paths configured for fault tolerance in Time-Sensitive Networking
// (TSN) applications.
//
simple FailureProtectionConfigurationCanvasVisualizer extends TreeVisualizerBase like IFailureProtectionConfigurationVisualizer
{
    parameters:
        string failureProtectionConfiguratorModule = default("failureProtectionConfigurator");
        string streamFilter = default("*");
        double zIndex = default(3); // Determines the drawing order of figures relative to other visualizers
        @class(FailureProtectionConfigurationCanvasVisualizer);
}

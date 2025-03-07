//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.power;

import inet.visualizer.base.EnergyStorageVisualizerBase;
import inet.visualizer.contract.IEnergyStorageVisualizer;

//
// Visualizes energy storage modules on a 2D canvas. Displays battery levels
// and power consumption information for network nodes.
//
// The visualization includes:
// - Colored bars representing the current energy level in storage devices
// - Customizable size, color, and placement of energy indicators
// - Automatic updates as energy levels change during simulation
// - Filtering options to visualize only specific energy storage modules
//
// Particularly useful for understanding power consumption patterns in wireless
// networks, analyzing battery lifetime in mobile devices, and debugging energy-aware
// protocols.
//
// @see ~EnergyStorageOsgVisualizer, ~EnergyStorageVisualizer, ~EnergyStorageVisualizerBase, ~IEnergyStorageVisualizer
//
simple EnergyStorageCanvasVisualizer extends EnergyStorageVisualizerBase like IEnergyStorageVisualizer
{
    parameters:
        double zIndex = default(10); // Determines the drawing order of figures relative to other visualizers
        @class(EnergyStorageCanvasVisualizer);
}

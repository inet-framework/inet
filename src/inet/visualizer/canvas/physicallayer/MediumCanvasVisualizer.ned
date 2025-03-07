//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.physicallayer;

import inet.visualizer.base.MediumVisualizerBase;
import inet.visualizer.contract.IMediumVisualizer;

//
// Visualizes radio medium activity on a 2D canvas. Displays propagating signals,
// transmission/reception indicators, and communication/interference ranges.
//
// The visualization includes:
// - Propagating signals as animated disks with opacity gradients
// - Signal departure indicators (blue) showing transmission power in dBW
// - Signal arrival indicators (red) showing reception power in dBW
// - Communication ranges as circles showing the maximum distance for successful reception
// - Interference ranges as circles showing the maximum distance where signals can cause interference
// - Overlapping signals to indicate potential collisions
//
// Particularly useful for understanding wireless signal propagation, analyzing
// transmission patterns, troubleshooting interference issues, and visualizing
// network coverage.
//
// @see ~MediumOsgVisualizer, ~MediumVisualizer, ~MediumVisualizerBase, ~IMediumVisualizer
//
simple MediumCanvasVisualizer extends MediumVisualizerBase like IMediumVisualizer
{
    parameters:
        double zIndex = default(0); // Determines the drawing order of figures relative to other visualizers

        // Signal visualization parameters
        string signalShape @enum("ring", "sphere") = default("ring"); // Shape used for displaying propagating signals
        double signalOpacity = default(0.5); // Opacity of signal visualizations, value must be in the range [0, 1]

        // Stationary ring parameters
        int signalRingCount = default(20); // Number of stationary rings displayed around the transmitter
        double signalRingSize @unit(m) = default(50m); // Width of each stationary ring

        // Signal fading parameters
        double signalFadingDistance @unit(m) = default(50m); // Distance parameter controlling how quickly signal opacity decreases
        double signalFadingFactor = default(1.2); // Factor parameter for exponential opacity decreasing, must be > 1

        // Wave visualization parameters
        int signalWaveCount = default(20); // Number of moving circles representing propagating waves
        double signalWaveLength @unit(m) = default(50m); // Distance between consecutive wave circles
        double signalWaveWidth @unit(m) = default(10m); // Width of each wave circle
        double signalWaveFadingAnimationSpeedFactor = default(1); // Controls animation speed of fading waves, range [0, 1]

        // Heat map visualization
        bool displayCommunicationHeat = default(false); // When enabled, displays recent successful receptions as a heat map overlay

        @class(MediumCanvasVisualizer);
}

//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.physicallayer;

import inet.visualizer.base.MediumVisualizerBase;
import inet.visualizer.contract.IMediumVisualizer;

//
// Visualizes radio medium activity in a 3D scene using OpenSceneGraph (OSG).
// Displays propagating signals, transmission/reception indicators, and
// communication/interference ranges.
//
// The visualization includes:
// - Propagating signals as animated rings or 3D spheres with configurable appearance
// - Signal departure indicators showing transmission activity
// - Signal arrival indicators showing reception activity
// - Communication ranges as circles showing the maximum distance for successful reception
// - Interference ranges as circles showing the maximum distance where signals can cause interference
// - Flexible placement options with support for different visualization planes (camera, xy, xz, yz)
//
// The visualizer offers extensive customization of signal appearance including:
// - Shape selection (ring, sphere, or both)
// - Wave properties (length, amplitude, fading)
// - Plane selection for 2D elements
// - Animation speed control
//
// Particularly useful for understanding wireless signal propagation in 3D space,
// analyzing transmission patterns, troubleshooting interference issues, and
// creating compelling visualizations of wireless communication.
//
// @see ~MediumCanvasVisualizer, ~MediumVisualizer, ~MediumVisualizerBase, ~IMediumVisualizer
//
simple MediumOsgVisualizer extends MediumVisualizerBase like IMediumVisualizer
{
    parameters:
        string signalShape @enum("ring", "sphere", "both") = default("ring"); // Shape for displaying signals
        string signalPlane @enum("camera","xy","xz","yz") = default("xy"); // Plane for 2-dimensional signal shapes
        double signalFadingDistance @unit(m) = default(100m); // Distance parameter for exponential opacity decreasing
        double signalFadingFactor = default(1.5); // Factor parameter for exponential opacity decreasing, value must be in the range (1.0, +inf)
        double signalWaveLength @unit(m) = default(100m); // Distance between signal waves, value must be in the range (0, +inf)
        double signalWaveAmplitude = default(0.5); // Relative opacity amplitude of signal waves, value must be in the range [0, 1]
        double signalWaveFadingAnimationSpeedFactor = default(1.0); // Value must be in the range [0, 1]

        string transmissionPlane @enum("camera","xy","xz","yz") = default("camera"); // Plane for transmission image
        string receptionPlane @enum("camera","xy","xz","yz") = default("camera"); // Plane for reception image

        string communicationRangePlane @enum("camera","xy","xz","yz") = default("xy"); // Plane for the transmission range circle
        string interferenceRangePlane @enum("camera","xy","xz","yz") = default("xy"); // Plane for the interference range circle

        @class(MediumOsgVisualizer);
}

//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.vsg.physicallayer;

import inet.visualizer.base.MediumVisualizerBase;
import inet.visualizer.contract.IMediumVisualizer;

//
// Visualizes a medium using a 3D VulkanSceneGraph (VSG) scene. It displays
// communication ranges and interference ranges as circles around network nodes.
// It displays a marker at network nodes which are currently transmitting, and
// a different marker at other network nodes which are currently receiving. It
// also displays radio signals as a growing 2D ring or a 3D sphere as they
// propagate through the medium. All 2D shapes such as circles and rings can be
// displayed in 4 different planes: camera, xy, xz, and yz.
//
// Note: This is the VSG port of ~MediumOsgVisualizer. Shader-based effects
// (wave-modulated opacity, signal-strength gradient rings, textured icons) are
// approximated with solid/translucent geometry; see the C++ source for TODO
// comments at each approximation site.
//
// @see ~MediumOsgVisualizer, ~MediumVisualizer, ~MediumVisualizerBase, ~IMediumVisualizer
//
simple MediumVsgVisualizer extends MediumVisualizerBase like IMediumVisualizer
{
    parameters:
        string signalShape @enum("ring", "sphere", "both") = default("ring"); // Shape for displaying signals
        string signalPlane @enum("camera","xy","xz","yz") = default("xy"); // Plane for 2-dimensional signal shapes
        double signalFadingDistance @unit(m) = default(100m); // Distance parameter for exponential opacity decreasing
        double signalFadingFactor = default(1.5); // Factor parameter for exponential opacity decreasing, value must be in the range (1.0, +inf)
        double signalWaveLength @unit(m) = default(100m); // Distance between signal waves, value must be in the range (0, +inf)
        double signalWaveAmplitude = default(0.5); // Relative opacity amplitude of signal waves, value must be in the range [0, 1]
        double signalWaveFadingAnimationSpeedFactor = default(1.0); // Value must be in the range [0, 1]
        bool signalWaveShader = default(false); // Experimental: render the ring wavefront with a GLSL fragment shader (per-pixel, smoothly flowing ripple) instead of baked per-vertex alpha

        string transmissionPlane @enum("camera","xy","xz","yz") = default("camera"); // Plane for transmission indicator
        string receptionPlane @enum("camera","xy","xz","yz") = default("camera"); // Plane for reception indicator

        string communicationRangePlane @enum("camera","xy","xz","yz") = default("xy"); // Plane for the transmission range circle
        string interferenceRangePlane @enum("camera","xy","xz","yz") = default("xy"); // Plane for the interference range circle

        @class(MediumVsgVisualizer);
}

//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.mobility;

import inet.visualizer.base.MobilityVisualizerBase;
import inet.visualizer.contract.IMobilityVisualizer;

//
// Visualizes node mobility on a 2D canvas. Displays movement trails, velocity vectors,
// and orientation indicators to help track and understand node movement.
//
// The visualization includes:
// - Movement trails showing the recent path of mobile nodes (with configurable fading)
// - Velocity vectors represented as arrows indicating speed and direction
// - Orientation arcs showing which way nodes are facing
// - Automatic updates as nodes move according to their mobility models
//
// Particularly useful for tracking node movement in wireless networks, analyzing
// mobility patterns, and verifying that mobility models are working as expected.
//
// @see ~MobilityOsgVisualizer, ~MobilityVisualizer, ~MobilityVisualizerBase
//
simple MobilityCanvasVisualizer extends MobilityVisualizerBase like IMobilityVisualizer
{
    parameters:
        double zIndex = default(0); // Determines the drawing order of figures relative to other visualizers
        @class(MobilityCanvasVisualizer);
}

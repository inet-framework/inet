//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.environment;

import inet.visualizer.base.PhysicalEnvironmentVisualizerBase;
import inet.visualizer.contract.IPhysicalEnvironmentVisualizer;

//
// Visualizes physical environments on a 2D canvas. Displays buildings, walls,
// and other physical objects that can affect signal propagation.
//
// The visualization includes:
// - 2D projections of 3D physical objects according to the view angle
// - Proper representation of object positions, orientations, and shapes
// - Visual properties like colors and opacity as defined in the environment
// - Automatic updates when objects change or move
//
// Particularly useful for understanding signal propagation in environments with
// obstacles, verifying object placement in simulations, and creating realistic
// visualizations of indoor or urban scenarios.
//
// @see ~PhysicalEnvironmentOsgVisualizer, ~PhysicalEnvironmentVisualizer, ~PhysicalEnvironmentVisualizerBase, ~IPhysicalEnvironmentVisualizer
//
simple PhysicalEnvironmentCanvasVisualizer extends PhysicalEnvironmentVisualizerBase like IPhysicalEnvironmentVisualizer
{
    parameters:
        double zIndex = default(0); // Determines the drawing order of figures relative to other visualizers
        @class(PhysicalEnvironmentCanvasVisualizer);
}

//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.environment;

import inet.visualizer.base.PhysicalEnvironmentVisualizerBase;
import inet.visualizer.contract.IPhysicalEnvironmentVisualizer;

//
// Visualizes physical environments in a 3D scene using OpenSceneGraph (OSG).
// Displays buildings, walls, and other physical objects that can affect signal
// propagation.
//
// The visualization includes:
// - 3D representations of physical objects
// - Proper positioning, orientation, and scaling of objects
// - Material properties including colors and transparency
// - Support for complex shapes and object hierarchies TODO?
// - Optional transparency for seeing inside buildings TODO?
//
// Particularly useful for understanding signal propagation in environments with
// obstacles, visualizing complex indoor or urban scenarios, analyzing coverage
// problems due to physical barriers, and creating complex 3D visualizations
// of network environments.
//
// @see ~PhysicalEnvironmentCanvasVisualizer, ~PhysicalEnvironmentVisualizer, ~PhysicalEnvironmentVisualizerBase, ~IPhysicalEnvironmentVisualizer
//
simple PhysicalEnvironmentOsgVisualizer extends PhysicalEnvironmentVisualizerBase like IPhysicalEnvironmentVisualizer
{
    parameters:
        @class(PhysicalEnvironmentOsgVisualizer);
        bool enableObjectOpacity = default(true); // Display physical objects potentially transparently as specified in the opacity attribute (might cause z-fighting for transparent objects)
}

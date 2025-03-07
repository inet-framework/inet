//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for physical environment visualizer simple modules. It
// visualizes physical objects such as buildings, walls, obstacles, and terrain
// features that affect signal propagation in wireless networks.
//
// By default, physical objects are displayed. To disable visualization, set the
// displayObjects parameter to false.
//
// The visualizer obtains physical objects from the physical environment module
// specified by the physicalEnvironmentModule parameter. These objects are defined
// in XML files and can represent various shapes (cuboids, spheres, prisms, etc.)
// with different materials that affect radio signal propagation.
//
// The actual rendering of physical objects is implemented in derived modules
// for specific user interfaces (canvas or OSG). The OSG visualizer provides
// particularly realistic 3D visualization of the physical environment.
//
// This visualizer is particularly useful for understanding how the physical
// environment affects wireless communication, analyzing signal propagation
// in indoor scenarios, and debugging obstacle loss models.
//
// @see ~PhysicalEnvironmentCanvasVisualizer, ~PhysicalEnvironmentOsgVisualizer, ~IPhysicalEnvironmentVisualizer, ~VisualizerBase
//
simple PhysicalEnvironmentVisualizerBase extends VisualizerBase
{
    parameters:
        string physicalEnvironmentModule = default("physicalEnvironment"); // The physical environment module that is displayed, refers to a top-level sub-module by default

        bool displayObjects = default(true);

        @class(PhysicalEnvironmentVisualizerBase);
}

//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for physical environment visualizers. Physical environment visualizers
// display objects such as buildings, walls, and other physical structures that
// affect signal propagation in the simulation.
//
// @see ~PhysicalEnvironmentCanvasVisualizer, ~PhysicalEnvironmentOsgVisualizer, ~PhysicalEnvironmentVisualizer, ~PhysicalEnvironmentVisualizerBase
//
moduleinterface IPhysicalEnvironmentVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}

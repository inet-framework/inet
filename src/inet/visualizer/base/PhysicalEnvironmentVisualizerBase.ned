//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for physical environment visualizer simple modules. It
// provides the physical objects through the physical environment module for
// derived modules. The actual visualization of physical objects is done in
// derived modules.
//
// @see ~PhysicalEnvironmentCanvasVisualizer, ~PhysicalEnvironmentOsgVisualizer, ~IPhysicalEnvironmentVisualizer, ~VisualizerBase
//
simple PhysicalEnvironmentVisualizerBase extends VisualizerBase
{
    parameters:
        string physicalEnvironmentModule = default("physicalEnvironment"); // the physical environment module that is displayed, refers to a top level submodule by default

        bool displayObjects = default(true);

        @class(PhysicalEnvironmentVisualizerBase);
}


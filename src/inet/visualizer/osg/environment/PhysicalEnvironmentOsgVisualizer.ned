//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.environment;

import inet.visualizer.base.PhysicalEnvironmentVisualizerBase;
import inet.visualizer.contract.IPhysicalEnvironmentVisualizer;

//
// This module visualizes a physical environment on a 3D osg scene. It displays
// the physical objects present in the physical environment. The visualizer
// considers all object properties that affect appearance such as position,
// orientation, shape, colors, opacity, etc.
//
// @see ~PhysicalEnvironmentOsgVisualizer, ~PhysicalEnvironmentVisualizer, ~PhysicalEnvironmentVisualizerBase, ~IPhysicalEnvironmentVisualizer
//
simple PhysicalEnvironmentOsgVisualizer extends PhysicalEnvironmentVisualizerBase like IPhysicalEnvironmentVisualizer
{
    parameters:
        @class(PhysicalEnvironmentOsgVisualizer);
        bool enableObjectOpacity = default(true); // display physical objects potentially transparently as specified in the opacitiy attribute (might cause z-fighting for transparent objects)
}


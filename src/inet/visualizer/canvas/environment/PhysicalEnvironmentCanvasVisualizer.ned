//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.environment;

import inet.visualizer.base.PhysicalEnvironmentVisualizerBase;
import inet.visualizer.contract.IPhysicalEnvironmentVisualizer;

//
// This module visualizes a physical environment on a 2D canvas. It displays the
// physical objects present in the physical environment. Since this is not a 3D
// visualization, so the physical objects are projected to a 2D plane according
// to the view angle of the corresponding ~SceneCanvasVisualizer. Apart from this
// the visualizer considers all object properties that affect the appearance
// such as position, orientation, shape, colors, opacity, etc.
//
// @see ~PhysicalEnvironmentOsgVisualizer, ~PhysicalEnvironmentVisualizer, ~PhysicalEnvironmentVisualizerBase, ~IPhysicalEnvironmentVisualizer
//
simple PhysicalEnvironmentCanvasVisualizer extends PhysicalEnvironmentVisualizerBase like IPhysicalEnvironmentVisualizer
{
    parameters:
        double zIndex = default(0); // determines the drawing order of figures relative to other visualizers
        @class(PhysicalEnvironmentCanvasVisualizer);
}


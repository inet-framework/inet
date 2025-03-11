//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.physicallayer;

import inet.visualizer.base.TracingObstacleLossVisualizerBase;
import inet.visualizer.contract.ITracingObstacleLossVisualizer;

// TODO
// Visualizes signal obstacle interactions in a 3D scene using OpenSceneGraph (OSG).
// Displays the points where wireless signals intersect with physical objects.
//
// The visualization includes:
// - Lines marking the exact intersection points of signals with obstacles
// - Normal vectors showing the orientation of object faces at intersection points
// - Visual indication of signal path obstructions
// - 3D representation of how signals interact with the physical environment
//
// Particularly useful for understanding signal propagation in complex environments,
// analyzing coverage problems due to obstacles, debugging obstacle loss models,
// and creating realistic visualizations of wireless signal behavior in the
// presence of physical barriers.
//
// @see ~TracingObstacleLossCanvasVisualizer, ~TracingObstacleLossVisualizer, ~TracingObstacleLossVisualizerBase, ~ITracingObstacleLossVisualizer
//
simple TracingObstacleLossOsgVisualizer extends TracingObstacleLossVisualizerBase like ITracingObstacleLossVisualizer
{
    parameters:
        @class(TracingObstacleLossOsgVisualizer);
}

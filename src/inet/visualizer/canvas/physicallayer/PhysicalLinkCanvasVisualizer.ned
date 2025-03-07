//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.physicallayer;

import inet.visualizer.base.PhysicalLinkVisualizerBase;
import inet.visualizer.contract.IPhysicalLinkVisualizer;

//
// Visualizes physical layer links on a 2D canvas. Displays arrows representing
// active communication at the physical layer between network nodes.
//
// The visualization includes:
// - Arrows pointing from transmitter to receiver nodes
// - Separate arrows for each direction when bidirectional communication occurs
// - Gradual fading of arrows as links become inactive
// - Dynamic updates when nodes move
// - Filtering options to visualize only specific packet types or nodes
//
// Particularly useful for understanding physical connectivity patterns,
// analyzing wireless transmission paths, and troubleshooting physical layer
// communication issues.
//
// @see ~PhysicalLinkOsgVisualizer, ~PhysicalLinkVisualizer, ~PhysicalLinkVisualizerBase, ~IPhysicalLinkVisualizer
//
simple PhysicalLinkCanvasVisualizer extends PhysicalLinkVisualizerBase like IPhysicalLinkVisualizer
{
    parameters:
        double zIndex = default(1); // Determines the drawing order of figures relative to other visualizers
        @class(PhysicalLinkCanvasVisualizer);
}

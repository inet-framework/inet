//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.physicallayer;

import inet.visualizer.base.PhysicalLinkVisualizerBase;
import inet.visualizer.contract.IPhysicalLinkVisualizer;

//
// This module visualizes physical layer links on a 2D canvas. It displays an
// arrow for each active link in the network. See the documentation of the base
// module for what constitutes an active link. The arrow points from the source
// towards the destination. If a link is used in both directions then there are
// separate arrows for them. Each arrow fades out over time as the link becomes
// inactive unless it is reinforced by another packet. The arrow automatically
// follows the movement of mobile nodes.
//
// @see ~PhysicalLinkOsgVisualizer, ~PhysicalLinkVisualizer, ~PhysicalLinkVisualizerBase, ~IPhysicalLinkVisualizer
//
simple PhysicalLinkCanvasVisualizer extends PhysicalLinkVisualizerBase like IPhysicalLinkVisualizer
{
    parameters:
        double zIndex = default(1); // determines the drawing order of figures relative to other visualizers
        @class(PhysicalLinkCanvasVisualizer);
}


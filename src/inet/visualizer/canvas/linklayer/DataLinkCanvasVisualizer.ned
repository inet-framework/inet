//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.linklayer;

import inet.visualizer.base.DataLinkVisualizerBase;
import inet.visualizer.contract.IDataLinkVisualizer;

//
// This module visualizes data link layer links on a 2D canvas. It displays an
// arrow for each active link in the network. See the documentation of the base
// module for what constitutes an active link. The arrow points from the source
// towards the destination. If a link is used in both directions then there are
// separate arrows for them. Each arrow fades out over time as the link becomes
// inactive unless it is reinforced by another packet. The arrow automatically
// follows the movement of mobile nodes.
//
// @see ~DataLinkOsgVisualizer, ~DataLinkVisualizer, ~DataLinkVisualizerBase, ~IDataLinkVisualizer
//
simple DataLinkCanvasVisualizer extends DataLinkVisualizerBase like IDataLinkVisualizer
{
    parameters:
        double zIndex = default(2); // determines the drawing order of figures relative to other visualizers
        @class(DataLinkCanvasVisualizer);
}


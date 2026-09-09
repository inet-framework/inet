//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.vsg.linklayer;

import inet.visualizer.base.DataLinkVisualizerBase;
import inet.visualizer.contract.IDataLinkVisualizer;

//
// Visualizes data link layer links on a 3D VulkanSceneGraph scene. It displays
// an arrow for each active link in the network. See the documentation of the
// base module for what constitutes an active link. The arrow points from the
// source towards the destination. If a link is used in both directions then
// there are separate arrows for them. Each arrow fades out over time as the
// link becomes inactive unless it is reinforced by another packet. The arrow
// automatically follows the movement of mobile nodes.
//
// @see ~DataLinkCanvasVisualizer, ~DataLinkVisualizer, ~DataLinkVisualizerBase, ~IDataLinkVisualizer
//
simple DataLinkVsgVisualizer extends DataLinkVisualizerBase like IDataLinkVisualizer
{
    parameters:
        @class(DataLinkVsgVisualizer);
}

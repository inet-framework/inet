//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.vsg.networklayer;

import inet.visualizer.base.PathVisualizerBase;
import inet.visualizer.contract.INetworkRouteVisualizer;

//
// Visualizes network routes on a 3D VulkanSceneGraph scene. It displays a
// polyline for each active route in the network. See the documentation of the
// base module for what constitutes an active route. Each polyline fades out
// over time until the route becomes inactive unless it is reinforced by another
// packet. The polyline automatically follows the movement of mobile nodes.
//
// @see ~NetworkRouteCanvasVisualizer, ~NetworkRouteVisualizer, ~PathVisualizerBase, ~INetworkRouteVisualizer
//
simple NetworkRouteVsgVisualizer extends PathVisualizerBase like INetworkRouteVisualizer
{
    parameters:
        @class(NetworkRouteVsgVisualizer);
}

//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.networklayer;

import inet.visualizer.base.PathVisualizerBase;
import inet.visualizer.contract.INetworkRouteVisualizer;

//
// This module visualizes network routes on a 3D osg scene. It displays an
// polyline for each active route in the network. See the documentation of the
// base module for what constitutes an active route. Each polyline fades out
// over time until the route becomes inactive unless it is reinforced by another
// packet. The polyline automatically follows the movement of mobile nodes.
//
// @see ~NetworkRouteCanvasVisualizer, ~NetworkRouteVisualizer, ~PathVisualizerBase, ~INetworkRouteVisualizer
//
simple NetworkRouteOsgVisualizer extends PathVisualizerBase like INetworkRouteVisualizer
{
    parameters:
        @class(NetworkRouteOsgVisualizer);
}


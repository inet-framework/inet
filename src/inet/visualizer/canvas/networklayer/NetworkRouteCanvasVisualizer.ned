//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.networklayer;

import inet.visualizer.base.PathVisualizerBase;
import inet.visualizer.contract.INetworkRouteVisualizer;

//
// This module visualizes network routes on a 2D canvas. It displays an
// polyline arrow for each active route in the network. See the documentation of
// the base module for what constitutes an active route. The arrow points from
// the source towards the destination. If a link is used in both directions then
// there are arrowheads on both ends of the arrow. Each polyline fades out over
// time until the route becomes inactive unless it is reinforced by another
// packet. The polyline automatically follows the movement of mobile nodes.
//
// @see ~NetworkRouteOsgVisualizer, ~NetworkRouteVisualizer, ~PathVisualizerBase, ~INetworkRouteVisualizer
//
simple NetworkRouteCanvasVisualizer extends PathVisualizerBase like INetworkRouteVisualizer
{
    parameters:
        double zIndex = default(3); // determines the drawing order of figures relative to other visualizers
        @class(NetworkRouteCanvasVisualizer);
}


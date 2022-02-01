//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.transportlayer;

import inet.visualizer.base.PathVisualizerBase;
import inet.visualizer.contract.ITransportRouteVisualizer;

//
// This module visualizes transport layer routes on a 2D canvas. It displays an
// polyline arrow for each active route in the network. See the documentation
// of the base module for what constitutes an active route. The arrow points
// from the source towards the destination. If a route is used in both directions
// then there are separate pathes for them. Each polyline fades out over time
// as the route becomes inactive unless it is reinforced by another packet. The
// polyline automatically follows the movement of mobile nodes.
//
// @see ~TransportRouteOsgVisualizer, ~TransportRouteVisualizer, ~PathVisualizerBase, ~ITransportRouteVisualizer
//
simple TransportRouteCanvasVisualizer extends PathVisualizerBase like ITransportRouteVisualizer
{
    parameters:
        double zIndex = default(4); // determines the drawing order of figures relative to other visualizers
        @class(TransportRouteCanvasVisualizer);
}


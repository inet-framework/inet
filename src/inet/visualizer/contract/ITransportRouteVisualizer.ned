//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for transport route visualizers. Transport route visualizers display
// the path that packets take through the network at the transport layer.
//
// @see ~TransportRouteCanvasVisualizer, ~TransportRouteOsgVisualizer, ~TransportRouteVisualizer
//
moduleinterface ITransportRouteVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}

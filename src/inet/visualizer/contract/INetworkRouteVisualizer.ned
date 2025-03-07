//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for network route visualizers. Network route visualizers display
// the path that packets take through the network at the network layer.
//
// @see ~NetworkRouteCanvasVisualizer, ~NetworkRouteOsgVisualizer, ~NetworkRouteVisualizer
//
moduleinterface INetworkRouteVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}

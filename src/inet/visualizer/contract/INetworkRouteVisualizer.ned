//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface allows configuring different ~INetworkRouteVisualizer
// submodules from INI files. Such a visualizer displays active network routes
// in a network.
//
// @see ~NetworkRouteCanvasVisualizer, ~NetworkRouteOsgVisualizer, ~NetworkRouteVisualizer
//
moduleinterface INetworkRouteVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}


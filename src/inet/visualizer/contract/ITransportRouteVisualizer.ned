//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// This module interface allows configuring different ~ITransportRouteVisualizer
// submodules from INI files. Such a visualizer displays active transport routes
// in a network.
//
// @see ~TransportRouteCanvasVisualizer, ~TransportRouteOsgVisualizer, ~TransportRouteVisualizer
//
moduleinterface ITransportRouteVisualizer
{
    parameters:
        @display("i=block/app");
}


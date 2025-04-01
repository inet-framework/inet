//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface allows configuring different ~INetworkConnectionVisualizer
// submodules from INI files. Such a visualizer creates the visual representations
// of network connections.
//
// @see ~NetworkConnectionCanvasVisualizer, ~NetworkConnectionOsgVisualizer, ~NetworkConnectionVisualizer, ~NetworkConnectionVisualizerBase
//
moduleinterface INetworkConnectionVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}


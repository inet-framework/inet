//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.visualizer.contract;

//
// Interface for packet flow visualizer modules that display packet flows through
// the network as animated paths. Allows configuring different packet flow visualizer
// implementations from INI files. These visualizers track and display the routes
// that packets take between network nodes, providing insight into network traffic
// patterns and protocol behavior.
//
moduleinterface IPacketFlowVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}


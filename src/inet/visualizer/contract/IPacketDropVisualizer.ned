//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface allows configuring different ~IPacketDropVisualizer
// submodules from INI files. Such a visualizer displays the packet drops
// in a network.
//
// @see ~PacketDropCanvasVisualizer, ~PacketDropOsgVisualizer, ~PacketDropVisualizer, ~PacketDropVisualizerBase
//
moduleinterface IPacketDropVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}


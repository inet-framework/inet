//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for packet drop visualizers. Packet drop visualizers display
// indicators at locations where packets are dropped during network simulation.
//
// @see ~PacketDropCanvasVisualizer, ~PacketDropOsgVisualizer, ~PacketDropVisualizer, ~PacketDropVisualizerBase
//
moduleinterface IPacketDropVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}

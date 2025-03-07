//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for packet flow visualizers. Packet flow visualizers display
// animated representations of packets as they travel through the network.
//
// @see ~PacketFlowCanvasVisualizer, ~PacketFlowOsgVisualizer, ~PacketFlowVisualizer, ~PacketFlowVisualizerBase
//
moduleinterface IPacketFlowVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}

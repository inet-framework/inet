//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for transport connection visualizers. Transport connection visualizers
// display active transport layer connections (TCP, UDP, SCTP) between network nodes.
//
// @see ~TransportConnectionCanvasVisualizer, ~TransportConnectionOsgVisualizer, ~TransportConnectionVisualizer, ~TransportConnectionVisualizerBase
//
moduleinterface ITransportConnectionVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}

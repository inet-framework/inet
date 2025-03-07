//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for channel visualizers. Channel visualizers display communication
// channels and their properties in wireless and wired networks.
//
// @see ~ChannelCanvasVisualizer, ~ChannelOsgVisualizer, ~ChannelVisualizer, ~ChannelVisualizerBase
//
moduleinterface IChannelVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}

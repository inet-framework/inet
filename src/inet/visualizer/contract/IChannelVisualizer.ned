//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.visualizer.contract;

//
// Interface for channel visualizer modules that display communication channels
// as arrows between network nodes. Allows configuring different channel visualizer
// implementations from INI files. Channel visualizers track packet flows and
// display animated arrows to represent active communication paths in the network.
//
moduleinterface IChannelVisualizer
{
    parameters:
        @display("i=block/app");
}


//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.common;

import inet.visualizer.base.PacketDropVisualizerBase;
import inet.visualizer.contract.IPacketDropVisualizer;

//
// This module visualizes packet drops on a 3D osg scene. It displays an icon for
// each packet drop in the network. See the documentation of the base module for
// what constitutes a packet drop.
//
// @see ~PacketDropOsgVisualizer, ~PacketDropVisualizer, ~PacketDropVisualizerBase, ~IPacketDropVisualizer
//
simple PacketDropOsgVisualizer extends PacketDropVisualizerBase like IPacketDropVisualizer
{
    parameters:
        @class(PacketDropOsgVisualizer);
}


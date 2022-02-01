//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.transportlayer;

import inet.visualizer.base.TransportConnectionVisualizerBase;
import inet.visualizer.contract.ITransportConnectionVisualizer;

//
// This module visualizes transport layer connections on a 3D osg scene.
// It displays a pair of colored icons for each connection at the network nodes
// where the connection starts or ends.
//
// @see ~TransportConnectionCanvasVisualizer, ~TransportConnectionVisualizer, ~TransportConnectionVisualizerBase, ~ITransportConnectionVisualizer
//
simple TransportConnectionOsgVisualizer extends TransportConnectionVisualizerBase like ITransportConnectionVisualizer
{
    parameters:
        @class(TransportConnectionOsgVisualizer);
}


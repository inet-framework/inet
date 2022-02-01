//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.mobility;

import inet.visualizer.base.MobilityVisualizerBase;
import inet.visualizer.contract.IMobilityVisualizer;

//
// This module visualizes multiple mobilities on a 3D osg scene. It moves the
// visual represnetation according to their mobility model. It also displays
// recent movement as a polyline, current velocity vector and current orientation.
//
// @see ~MobilityCanvasVisualizer, ~MobilityVisualizer, ~MobilityVisualizerBase, ~IMobilityVisualizer
//
simple MobilityOsgVisualizer extends MobilityVisualizerBase like IMobilityVisualizer
{
    parameters:
        @class(MobilityOsgVisualizer);
}


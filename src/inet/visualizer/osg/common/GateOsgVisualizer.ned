//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.common;

import inet.visualizer.base.GateVisualizerBase;
import inet.visualizer.contract.IGateVisualizer;

//
// This module visualizes gates on a 3D osg scene.
//
// @see ~GateCanvasVisualizer, ~GateVisualizer, ~GateVisualizerBase, ~IGateVisualizer
//
simple GateOsgVisualizer extends GateVisualizerBase like IGateVisualizer
{
    parameters:
        @class(GateOsgVisualizer);
}


//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.linklayer;

import inet.visualizer.base.InterfaceTableVisualizerBase;
import inet.visualizer.contract.IInterfaceTableVisualizer;

//
// This module visualizes interface tables on a 2D canvas. It displays a label
// for each selected network interface at the owner network node.
//
// @see ~InterfaceTableOsgVisualizer, ~InterfaceTableVisualizer, ~InterfaceTableVisualizerBase, ~IInterfaceTableVisualizer
//
simple InterfaceTableCanvasVisualizer extends InterfaceTableVisualizerBase like IInterfaceTableVisualizer
{
    parameters:
        double zIndex = default(10); // determines the drawing order of figures relative to other visualizers
        @class(InterfaceTableCanvasVisualizer);
}


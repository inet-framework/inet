//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.linklayer;

import inet.visualizer.base.InterfaceTableVisualizerBase;
import inet.visualizer.contract.IInterfaceTableVisualizer;

//
// This module visualizes interface tables on a 3D osg scene. It displays a
// label for each selected network interface at the owner network node.
//
// @see ~InterfaceTableCanvasVisualizer, ~InterfaceTableVisualizer, ~InterfaceTableVisualizerBase, ~IInterfaceTableVisualizer
//
simple InterfaceTableOsgVisualizer extends InterfaceTableVisualizerBase like IInterfaceTableVisualizer
{
    parameters:
        @class(InterfaceTableOsgVisualizer);
}


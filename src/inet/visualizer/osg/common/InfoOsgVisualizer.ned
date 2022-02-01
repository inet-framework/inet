//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.common;

import inet.visualizer.base.InfoVisualizerBase;
import inet.visualizer.contract.IInfoVisualizer;

//
// This module visualizes some information of a submodule for each network node
// on a 3D osg scene.
//
// @see ~InfoCanvasVisualizer, ~InfoVisualizer, ~InfoVisualizerBase, ~IInfoVisualizer
//
simple InfoOsgVisualizer extends InfoVisualizerBase like IInfoVisualizer
{
    parameters:
        @class(InfoOsgVisualizer);
}


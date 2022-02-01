//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.linklayer;

import inet.visualizer.base.LinkBreakVisualizerBase;
import inet.visualizer.contract.ILinkBreakVisualizer;

//
// This module visualizes link breaks on a 3D osg scene. It displays an icon
// for each link break in the network. See the documentation of the base
// module for what constitutes a link break. The icon is positioned at the
// middle point of the link.
//
// @see ~LinkBreakCanvasVisualizer, ~LinkBreakVisualizer, ~LinkBreakVisualizerBase, ~ILinkBreakVisualizer
//
simple LinkBreakOsgVisualizer extends LinkBreakVisualizerBase like ILinkBreakVisualizer
{
    parameters:
        @class(LinkBreakOsgVisualizer);
}


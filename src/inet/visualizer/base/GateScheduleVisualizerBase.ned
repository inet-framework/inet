//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for gate visualizer simple modules.
//
// Format string directives:
//	- %i: interface name
// 	- %m: gate module name
// 	- %d: gate module display name
//  - %D: gate module display name if exists, otherwise gate module name
//
// @see ~GateScheduleCanvasVisualizer, ~GateScheduleOsgVisualizer, ~IGateScheduleVisualizer, ~VisualizerBase
//
simple GateScheduleVisualizerBase extends VisualizerBase
{
    parameters:
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // module path to the network node visualizer module, refers to a sibling submodule by default

        bool displayGateSchedules = default(false);
        string labelFormat = default("%i %D");

        string gateFilter = default("**"); // which gates are considered, matches all gates by default

        double width = default(100);
        double height = default(8);
        double spacing = default(2);
        string placementHint = default("right"); // annotation placement hint, space separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double placementPriority = default(0); // determines the order of annotation positioning

        double displayDuration @mutable @unit(s) = default(0s); // gate operation time duration visualized in the amount of space available, use gate period by default
        double currentTimePosition = default(width / 2); // position of the current simulation time in the visualization, horizontal center by default

        @class(GateScheduleVisualizerBase);
}


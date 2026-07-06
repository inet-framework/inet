//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface allows configuring different ~IGeoHorizonVisualizer submodules from
// INI files. Such a visualizer draws the satellite visibility footprint (the area
// on the Earth from which a node is seen above an elevation mask) on a 2D map.
//
// @see ~GeoHorizonCanvasVisualizer
//
moduleinterface IGeoHorizonVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
        string mapVisualizer;
}

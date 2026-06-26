//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface allows configuring different ~IGeoSkyViewVisualizer
// submodules from INI files. Such a visualizer draws an azimuth/elevation sky
// view polar plot next to each observer node, showing where the target nodes
// (e.g. satellites) are in the observer's sky.
//
// @see ~GeoSkyViewCanvasVisualizer
//
moduleinterface IGeoSkyViewVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}

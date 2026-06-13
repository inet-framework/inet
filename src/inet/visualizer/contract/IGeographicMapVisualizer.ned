//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface allows configuring different ~IGeographicMapVisualizer
// submodules from INI files. Such a visualizer projects geographically located
// network nodes onto a 2D world map.
//
// @see ~GeographicMapCanvasVisualizer, ~GeographicMapVisualizerBase
//
moduleinterface IGeographicMapVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}

//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for data link visualizers. Data link visualizers display active
// connections between network nodes at the data link layer (Layer 2).
//
// @see ~DataLinkCanvasVisualizer, ~DataLinkOsgVisualizer, ~DataLinkVisualizer, ~DataLinkVisualizerBase
//
moduleinterface IDataLinkVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}

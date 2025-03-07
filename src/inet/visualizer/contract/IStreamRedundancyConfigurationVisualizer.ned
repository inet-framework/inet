//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for stream redundancy configuration visualizers. Stream redundancy
// configuration visualizers display redundant paths for data streams in
// Time-Sensitive Networking (TSN) applications.
//
// @see ~StreamRedundancyConfigurationCanvasVisualizer, ~StreamRedundancyConfigurationOsgVisualizer, ~StreamRedundancyConfigurationVisualizer
//
moduleinterface IStreamRedundancyConfigurationVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}

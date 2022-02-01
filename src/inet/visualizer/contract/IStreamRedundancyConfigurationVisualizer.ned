//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// This module interface allows configuring different ~IStreamRedundancyConfigurationVisualizer
// submodules from INI files. Such a visualizer displays the StreamRedundancy configuration
// in a network.
//
// @see ~StreamRedundancyConfigurationCanvasVisualizer, ~StreamRedundancyConfigurationOsgVisualizer, ~StreamRedundancyConfigurationVisualizer, ~StreamRedundancyConfigurationVisualizerBase
//
moduleinterface IStreamRedundancyConfigurationVisualizer
{
    parameters:
        @display("i=block/app");
}


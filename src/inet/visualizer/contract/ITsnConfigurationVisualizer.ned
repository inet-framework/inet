//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// This module interface allows configuring different ~IFailureProtectionConfigurationVisualizer
// submodules from INI files. Such a visualizer displays the TSN configuration
// in a network.
//
// @see ~FailureProtectionConfigurationCanvasVisualizer, ~FailureProtectionConfigurationOsgVisualizer, ~FailureProtectionConfigurationVisualizer, ~FailureProtectionConfigurationVisualizerBase
//
moduleinterface IFailureProtectionConfigurationVisualizer
{
    parameters:
        @display("i=block/app");
}


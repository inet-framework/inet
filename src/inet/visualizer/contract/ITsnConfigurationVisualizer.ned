//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for failure protection configuration visualizers. Failure protection
// configuration visualizers display the redundant paths configured for
// network resilience.
//
// @see ~FailureProtectionConfigurationCanvasVisualizer, ~FailureProtectionConfigurationOsgVisualizer, ~FailureProtectionConfigurationVisualizer
//
moduleinterface IFailureProtectionConfigurationVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}

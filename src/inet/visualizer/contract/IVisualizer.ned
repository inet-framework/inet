//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Base interface for all visualizer module interfaces in the INET Framework.
// Defines common parameters for specifying visualization target and subject
// modules. Visualizers provide graphical representation of various aspects of
// network simulation, such as links, traffic flows, routing paths, and network
// state information.
//
moduleinterface IVisualizer
{
    parameters:
        string visualizationTargetModule;
        string visualizationSubjectModule;
        @display("i=block/app");
}


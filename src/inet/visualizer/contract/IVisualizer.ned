//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

moduleinterface IVisualizer
{
    parameters:
        string visualizationTargetModule;
        string visualizationSubjectModule;
        @display("i=block/app");
}


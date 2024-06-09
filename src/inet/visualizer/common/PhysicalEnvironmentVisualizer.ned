//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.common;

import inet.visualizer.contract.IPhysicalEnvironmentVisualizer;

//
// This module integrates canvas and OSG physical environment visualizers into a single module.
//
// @see ~PhysicalEnvironmentCanvasVisualizer, ~PhysicalEnvironmentOsgVisualizer, ~PhysicalEnvironmentVisualizerBase, ~IPhysicalEnvironmentVisualizer
//
module PhysicalEnvironmentVisualizer like IPhysicalEnvironmentVisualizer
{
    parameters:
        @display("i=block/app");
        string visualizationTargetModule = default("^");
        string visualizationSubjectModule = default("^");
        *.visualizationTargetModule = default(absPath(this.visualizationTargetModule));
        *.visualizationSubjectModule = default(absPath(this.visualizationSubjectModule));
    submodules:
        canvasVisualizer: <default(firstAvailableOrEmpty("PhysicalEnvironmentCanvasVisualizer"))> like IPhysicalEnvironmentVisualizer if typename != "" {
            parameters:
                @display("p=100,50");
        }
        osgVisualizer: <default(firstAvailableOrEmpty("PhysicalEnvironmentOsgVisualizer"))> like IPhysicalEnvironmentVisualizer if typename != "" {
            parameters:
                @display("p=100,150");
        }
}


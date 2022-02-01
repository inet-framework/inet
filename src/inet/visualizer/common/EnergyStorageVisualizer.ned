//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.common;

import inet.visualizer.contract.IEnergyStorageVisualizer;

//
// This module integrates canvas and osg energyStorage visualizers into a single module.
//
// @see ~EnergyStorageCanvasVisualizer, ~EnergyStorageOsgVisualizer, ~EnergyStorageVisualizerBase, ~IEnergyStorageVisualizer
//
module EnergyStorageVisualizer like IEnergyStorageVisualizer
{
    parameters:
        @display("i=block/app");
        *.visualizationTargetModule = default("^.^");
        *.visualizationSubjectModule = default("^.^");
    submodules:
        canvasVisualizer: <default(firstAvailableOrEmpty("EnergyStorageCanvasVisualizer"))> like IEnergyStorageVisualizer if typename != "" {
            parameters:
                @display("p=100,100");
        }
        osgVisualizer: <default(firstAvailableOrEmpty("EnergyStorageOsgVisualizer"))> like IEnergyStorageVisualizer if typename != "" {
            parameters:
                @display("p=100,200");
        }
}


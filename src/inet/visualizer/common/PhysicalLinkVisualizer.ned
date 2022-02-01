//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.common;

import inet.visualizer.contract.IPhysicalLinkVisualizer;

//
// This module integrates canvas and osg data link visualizers into a single module.
//
// @see ~PhysicalLinkCanvasVisualizer, ~PhysicalLinkOsgVisualizer, ~LinkVisualizerBase, ~IPhysicalLinkVisualizer
//
module PhysicalLinkVisualizer like IPhysicalLinkVisualizer
{
    parameters:
        @display("i=block/app");
        *.visualizationTargetModule = default("^.^");
        *.visualizationSubjectModule = default("^.^");
    submodules:
        canvasVisualizer: <default(firstAvailableOrEmpty("PhysicalLinkCanvasVisualizer"))> like IPhysicalLinkVisualizer if typename != "" {
            parameters:
                @display("p=100,100");
        }
        osgVisualizer: <default(firstAvailableOrEmpty("PhysicalLinkOsgVisualizer"))> like IPhysicalLinkVisualizer if typename != "" {
            parameters:
                @display("p=100,200");
        }
}


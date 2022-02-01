//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.common;

import inet.visualizer.contract.IDataLinkVisualizer;

//
// This module integrates canvas and osg data link visualizers into a single module.
//
// @see ~DataLinkCanvasVisualizer, ~DataLinkOsgVisualizer, ~LinkVisualizerBase, ~IDataLinkVisualizer
//
module DataLinkVisualizer like IDataLinkVisualizer
{
    parameters:
        @display("i=block/app");
        *.visualizationTargetModule = default("^.^");
        *.visualizationSubjectModule = default("^.^");
    submodules:
        canvasVisualizer: <default(firstAvailableOrEmpty("DataLinkCanvasVisualizer"))> like IDataLinkVisualizer if typename != "" {
            parameters:
                @display("p=100,100");
        }
        osgVisualizer: <default(firstAvailableOrEmpty("DataLinkOsgVisualizer"))> like IDataLinkVisualizer if typename != "" {
            parameters:
                @display("p=100,200");
        }
}


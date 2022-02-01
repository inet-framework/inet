//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.common;

import inet.visualizer.contract.IInterfaceTableVisualizer;

module InterfaceTableVisualizer like IInterfaceTableVisualizer
{
    parameters:
        @display("i=block/app");
        *.visualizationTargetModule = default("^.^");
        *.visualizationSubjectModule = default("^.^");
    submodules:
        canvasVisualizer: <default(firstAvailableOrEmpty("InterfaceTableCanvasVisualizer"))> like IInterfaceTableVisualizer if typename != "" {
            parameters:
                @display("p=100,100");
        }
        osgVisualizer: <default(firstAvailableOrEmpty("InterfaceTableOsgVisualizer"))> like IInterfaceTableVisualizer if typename != "" {
            parameters:
                @display("p=100,200");
        }
}


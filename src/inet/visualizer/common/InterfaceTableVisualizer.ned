//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.visualizer.common;

import inet.visualizer.contract.IInterfaceTableVisualizer;

//
// Visualizes network interface information from interface tables in the network.
// This compound module integrates both canvas (2D) and OSG (3D) visualizers,
// automatically selecting the appropriate one based on the runtime environment.
// It displays interface data such as addresses, state, and traffic statistics
// for network nodes.
//
module InterfaceTableVisualizer like IInterfaceTableVisualizer
{
    parameters:
        @display("i=block/app");
        string visualizationTargetModule = default("^");
        string visualizationSubjectModule = default("^");
        *.visualizationTargetModule = default(absPath(this.visualizationTargetModule));
        *.visualizationSubjectModule = default(absPath(this.visualizationSubjectModule));
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


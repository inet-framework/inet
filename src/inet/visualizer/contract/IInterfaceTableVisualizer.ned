//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for interface table visualizers. Interface table visualizers display
// information about network interfaces such as names, addresses, and states.
//
// @see ~InterfaceTableCanvasVisualizer, ~InterfaceTableOsgVisualizer, ~InterfaceTableVisualizer, ~InterfaceTableVisualizerBase
//
moduleinterface IInterfaceTableVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}

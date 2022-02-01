//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// This module interface allows configuring different ~IInterfaceTableVisualizer
// submodules from INI files. Such a visualizer displays various information
// for network interfaces present in the interface tables of a network.
//
// @see ~InterfaceTableCanvasVisualizer, ~InterfaceTableOsgVisualizer, ~InterfaceTableVisualizer, ~InterfaceTableVisualizerBase
//
moduleinterface IInterfaceTableVisualizer
{
    parameters:
        @display("i=block/app");
}


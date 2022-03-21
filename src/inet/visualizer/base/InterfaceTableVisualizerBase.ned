//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for interface table visualizer simple modules.
// It displays information of network interfaces at network nodes. Network
// interfaces are not displayed by default, see module parameters for how to
// enable the visualization.
//
// This module subscribes for interfaceCreated, interfaceDeleted,
// interfaceConfigChanged, and interfaceIpv4ConfigChanged on the
// module determined by the visualizationSubjectModule parameter.
//
// The format string can contain the following directives:
//  - %N interface name
//  - %m MAC address
//  - %4 IPv4 address
//  - %6 Ipv6 address
//  - %a IPv4 or IPv6 address
//  - %l IPv4 or IPv6 netmask length
//  - %g generic address
//  - %n network address
//  - %t up or down state
//  - %i NetworkInterface::info()
//  - %s NetworkInterface::str()
//  - %\ conditional newline for wired interfaces
//  - %/ conditional / for network address
//
// @see ~InterfaceTableCanvasVisualizer, ~InterfaceTableOsgVisualizer, ~IInterfaceTableVisualizer, ~VisualizerBase
//
simple InterfaceTableVisualizerBase extends VisualizerBase
{
    parameters:
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // module path to the network node visualizer module, refers to a sibling submodule by default

        bool displayInterfaceTables = default(false); // display interface table data, disabled by default
        bool displayWiredInterfacesAtConnections = default(true); // specifies where wired interfaces are displayed
        bool displayBackground = default(false); // specifies whether the text should have a background or not

        string nodeFilter @mutable = default("*"); // which nodes are considered, matches all nodes by default
        string interfaceFilter @mutable = default("not(lo*)"); // which interfaces are considered, matches all non-local interfaces by default

        string format @mutable = default("%N %\\%a%/%l"); // determines what interface data is displayed
        string placementHint = default("top"); // annotation placement hint, space separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double placementPriority = default(0); // determines the order of annotation positioning

        string font = default("<default>"); // interface text font, automatic by default
        string textColor = default("#005030"); // interface text font color
        string backgroundColor = default("white"); // interface text background color
        double opacity = default(1.0); // opacity of the visualization

        @class(InterfaceTableVisualizerBase);
}


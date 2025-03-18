//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// Base module for interface table visualizer simple modules.
// It displays information of network interfaces at network nodes. Network
// interfaces are not displayed by default, see module parameters for how to
// enable the visualization.
//
// This module subscribes to `interfaceCreated`, `interfaceDeleted`,
// `interfaceConfigChanged`, and `interfaceIpv4ConfigChanged` on the
// module determined by the `visualizationSubjectModule` parameter.
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
//  - %i the result of the `NetworkInterface::info()` call
//  - %s the result of the `NetworkInterface::str()` call
//  - %\ conditional newline for wired interfaces
//  - %/ conditional / for network address
//
// @see ~InterfaceTableCanvasVisualizer, ~InterfaceTableOsgVisualizer, ~IInterfaceTableVisualizer, ~VisualizerBase
//
simple InterfaceTableVisualizerBase extends VisualizerBase
{
    parameters:
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // Module path to the network node visualizer module, refers to a sibling submodule by default

        bool displayInterfaceTables = default(false); // Display interface table data, disabled by default
        bool displayWiredInterfacesAtConnections = default(true); // Specifies where wired interfaces are displayed
        bool displayBackground = default(false); // Specifies whether the text should have a background or not

        string nodeFilter @mutable = default("*"); // Which nodes are considered, matches all nodes by default
        string interfaceFilter @mutable = default("not(lo*)"); // Which interfaces are considered, matches all non-local interfaces by default

        string format @mutable = default("%N %\\%a%/%l"); // Determines what interface data is displayed
        string placementHint = default("top"); // Annotation placement hint, space separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double placementPriority = default(0); // Determines the order of annotation positioning

        string font = default("<default>"); // Interface text font, automatic by default
        string textColor = default("#005030"); // Interface text font color
        string backgroundColor = default("white"); // Interface text background color
        double opacity = default(1.0); // Opacity of the visualization

        @class(InterfaceTableVisualizerBase);
}


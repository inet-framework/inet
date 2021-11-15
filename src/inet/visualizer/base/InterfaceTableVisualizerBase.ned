//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
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
//  - %i InterfaceEntry::info()
//  - %s InterfaceEntry::str()
//  - %\ conditional newline for wired interfaces
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

        string format @mutable = default("%N %\\%a/%l"); // determines what interface data is displayed
        string placementHint = default("top"); // annotation placement hint, space separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double placementPriority = default(0); // determines the order of annotation positioning

        string font = default("<default>"); // interface text font, automatic by default
        string textColor = default("#005030"); // interface text font color
        string backgroundColor = default("white"); // interface text background color
        double opacity = default(1.0); // opacity of the visualization

        @class(InterfaceTableVisualizerBase);
}


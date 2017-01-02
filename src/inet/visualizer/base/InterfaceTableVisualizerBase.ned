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
// This module subscribes for NF_INTERFACE_CREATED, NF_INTERFACE_DELETED, 
// NF_INTERFACE_CONFIG_CHANGED, and NF_INTERFACE_IPv4CONFIG_CHANGED on the
// module determined by the subscriptionModule parameter.
//
// @see ~InterfaceTableCanvasVisualizer, ~InterfaceTableOsgVisualizer, ~IInterfaceTableVisualizer, ~VisualizerBase
//
simple InterfaceTableVisualizerBase extends VisualizerBase
{
    parameters:
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // module path to the network node visualizer module, refers to a sibling submodule by default
        string subscriptionModule = default("<root>"); // module where the visualizer subscribes for interface changed signals, root module by default
        
        bool displayInterfaceTables = default(false); // display interface table data, disabled by default

        string nodeFilter = default("*"); // which nodes are considered, matches all nodes by default
        string interfaceFilter = default("*"); // which interfaces are considered, matches all interfaces by default

        string content @enum("networkAddress", "macAddress", "info", "str") = default("networkAddress"); // determines what interface data is displayed 

        string font = default("<default>"); // interface text font, automatic by default
        string textColor = default("black"); // interface text font color
        string backgroundColor = default("yellow"); // interface text background color
        double opacity = default(1.0); // opacity of the visualization

        @class(InterfaceTableVisualizerBase);
}

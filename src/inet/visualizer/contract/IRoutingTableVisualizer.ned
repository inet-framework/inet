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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

package inet.visualizer.contract;

//
// This module interface allows configuring different ~IRoutingTableVisualizer
// submodules from INI files. Such a visualizer displays routes present in the
// routing tables of a network.
//
// @see ~RoutingTableCanvasVisualizer, ~RoutingTableOsgVisualizer, ~RoutingTableVisualizer, ~RoutingTableVisualizerBase
//
moduleinterface IRoutingTableVisualizer
{
    parameters:
        @display("i=block/app");
}


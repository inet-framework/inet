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
// This is a base module for transport connection visualizer simple modules. It
// keeps track of open transport connections in a network. Open connections are
// not displayed by default, see the module parameters for how to enable the
// visualization. The actual visualization of transport connections is done
// in derived modules.
//
// This module subscribes for tcpConnectionAdded and tcpConnectionRemoved on the
// module determined by the subscriptionModule parameter.
//
// @see ~TransportConnectionCanvasVisualizer, ~TransportConnectionOsgVisualizer, ~ITransportConnectionVisualizer, ~TransportConnectionVisualizerBase
//
simple TransportConnectionVisualizerBase extends VisualizerBase
{
    parameters:
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer");
        string subscriptionModule = default(""); // module path where the visualizer subscribes for connection signals, root module by default
        string nodeFilter = default(""); // which network nodes are considered to display connections, matches no nodes by default
        string icon = default("misc/marker_s"); // transport connection visualization icon
        @class(TransportConnectionVisualizerBase);
}

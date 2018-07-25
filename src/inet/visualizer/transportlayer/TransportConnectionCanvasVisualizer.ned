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

package inet.visualizer.transportlayer;

import inet.visualizer.base.TransportConnectionVisualizerBase;
import inet.visualizer.contract.ITransportConnectionVisualizer;

//
// This module visualizes transport layer connections on a 2D canvas. 
// It displays a pair of colored icons for each connection at the network nodes
// where the connection starts or ends.
//
// @see ~TransportConnectionOsgVisualizer, ~TransportConnectionVisualizer, ~TransportConnectionVisualizerBase, ~ITransportConnectionVisualizer
//
simple TransportConnectionCanvasVisualizer extends TransportConnectionVisualizerBase like ITransportConnectionVisualizer
{
    parameters:
        double zIndex = default(4); // determines the drawing order of figures relative to other visualizers
        @class(TransportConnectionCanvasVisualizer);
}


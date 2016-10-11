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

package inet.visualizer.physicallayer;

import inet.visualizer.base.MediumVisualizerBase;
import inet.visualizer.contract.IMediumVisualizer;

//
// This module visualizes a medium on a 2D canvas. It displays communication
// ranges and interference ranges as circles around network nodes. It displays
// an image at network nodes which are currently transmitting, and a different
// image at other network nodes whcih are currently receiving. It also displays
// radio signals as a 2D ring or a 3D sphere as they propagate through the
// medium.
//
// @see ~MediumCanvasVisualizer, ~MediumVisualizer, ~MediumVisualizerBase, ~IMediumVisualizer
//
simple MediumCanvasVisualizer extends MediumVisualizerBase like IMediumVisualizer
{
    parameters:
        @class(MediumCanvasVisualizer);
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // module path of the network node visualizer module
        double zIndex = default(0); // determines the drawing order of figures relative to other visualizers
        string signalShape @enum("ring", "sphere") = default("ring"); // shape for displaying signals
        bool displayCommunicationHeat = default(false); // when enabled the radio medium displays recent successful receptions as a heat map overlay
}

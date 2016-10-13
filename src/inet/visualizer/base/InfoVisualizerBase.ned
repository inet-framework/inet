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
// This is a base module for module information visualizer simple modules.
// It displays some information on network nodes originating from one of the
// corresponding submodules. No information is displayed by default, see module
// parameters for how to enable the visualization. The actual visualization is
// done in derived modules.
//
// @see ~InfoCanvasVisualizer, ~InfoOsgVisualizer, ~IInfoDropVisualizer, ~InfoVisualizerBase
//
simple InfoVisualizerBase extends VisualizerBase
{
    parameters:
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer");
        string modules = default(""); // specifies the submodules of network nodes, no submodules by default
        string content @enum("info", "displayStringText") = default("info"); // determines what is displayed on network nodes, info by default 
        string fontColor = default("black"); // info text font color, black by default
        string backgroundColor = default("white"); // info text background color, white by default
        double opacity = default(0.5); // opacity of the visualization
        @class(InfoVisualizerBase);
}

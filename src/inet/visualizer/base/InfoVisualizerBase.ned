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
// This is a base module for submodule information visualizer simple modules.
// It displays some information on network nodes originating from one of their
// corresponding submodules. No information is displayed by default, see module
// parameters for how to enable the visualization. The actual visualization is
// done in derived modules.
//
// The format string can contain the following directives:
//  - %n module full name
//  - %p module full path
//  - %t display string text
//  - %s cModule::str()
//
// @see ~InfoCanvasVisualizer, ~InfoOsgVisualizer, ~IInfoVisualizer, ~VisualizerBase
//
simple InfoVisualizerBase extends VisualizerBase
{
    parameters:
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // module path to the network node visualizer module, refers to a sibling submodule by default

        bool displayInfos = default(true);

        string modules = default(""); // specifies the path of the submodules of network nodes, no submodules by default
        string format = default("%t"); // determines what is displayed on network nodes 

        string font = default("<default>"); // info text font, automatic by default
        string textColor = default("black"); // info text font color, black by default
        string backgroundColor = default("white"); // info text background color, white by default
        double opacity = default(1.0); // opacity of the visualization
        string placementHint = default("top"); // annotation placement hint, space separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double placementPriority = default(0); // determines the order of annotation positioning

        @class(InfoVisualizerBase);
}


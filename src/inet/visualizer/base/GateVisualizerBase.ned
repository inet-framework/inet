//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.visualizer.base;

//
// This is a base module for gate visualizer simple modules.
//
// @see ~GateCanvasVisualizer, ~GateOsgVisualizer, ~IGateVisualizer, ~VisualizerBase
//
simple GateVisualizerBase extends VisualizerBase
{
    parameters:
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // module path to the network node visualizer module, refers to a sibling submodule by default

        bool displayGates = default(false);

        string gateFilter = default("**"); // which gates are considered, matches all gates by default

        double width = default(100);
        double height = default(8);
        double spacing = default(2);
        string placementHint = default("right"); // annotation placement hint, space separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double placementPriority = default(0); // determines the order of annotation positioning

        double displayDuration @mutable @unit(s) = default(0s); // gate operation time duration visualized in the amount of space available, use gate period by default
        double currentTimePosition = default(width / 2); // position of the current simulation time in the visualization, horizontal center by default

        @class(GateVisualizerBase);
}


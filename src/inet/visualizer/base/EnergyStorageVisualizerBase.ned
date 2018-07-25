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
// This is a base module for energy storage visualizer simple modules.
//
// @see ~EnergyStorageCanvasVisualizer, ~EnergyStorageOsgVisualizer, ~IEnergyStorageVisualizer, ~VisualizerBase
//
simple EnergyStorageVisualizerBase extends VisualizerBase
{
    parameters:
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // module path to the network node visualizer module, refers to a sibling submodule by default

        bool displayEnergyStorages = default(false);

        string energyStorageFilter = default("**"); // which energy storages are considered, matches all energy storages by default

        string color = default("green");
        double width = default(8);
        double height = default(24);
        double spacing = default(2);
        string placementHint = default("right"); // annotation placement hint, space separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double placementPriority = default(0); // determines the order of annotation positioning

        @class(EnergyStorageVisualizerBase);
}


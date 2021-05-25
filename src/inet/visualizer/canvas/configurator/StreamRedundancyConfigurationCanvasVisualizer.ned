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

package inet.visualizer.canvas.configurator;

import inet.visualizer.base.TreeVisualizerBase;
import inet.visualizer.contract.IStreamRedundancyConfigurationVisualizer;

simple StreamRedundancyConfigurationCanvasVisualizer extends TreeVisualizerBase like IStreamRedundancyConfigurationVisualizer
{
    parameters:
        string streamRedundancyConfiguratorModule = default("streamRedundancyConfigurator");
        string streamFilter = default("*");
        double zIndex = default(3); // determines the drawing order of figures relative to other visualizers
        @class(StreamRedundancyConfigurationCanvasVisualizer);
}


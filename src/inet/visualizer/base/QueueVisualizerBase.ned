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
// This is a base module for queue visualizer simple modules.
//
// @see ~QueueCanvasVisualizer, ~QueueOsgVisualizer, ~IQueueVisualizer, ~VisualizerBase
//
simple QueueVisualizerBase extends VisualizerBase
{
    parameters:
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // module path to the network node visualizer module, refers to a sibling submodule by default

        bool displayQueues = default(false);

        string queueFilter = default("**"); // which queues are considered, matches no queues by default

        string color = default("blue");
        double spacing = default(2);
        double elementWidth = default(8);
        double elementHeight = default(4);
        int elementCount = default(-1); // specifies the number of elements, queue length by default
        string displacement = default("right");

        @class(QueueVisualizerBase);
}

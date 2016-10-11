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
// This is a base module for tracing obstacle loss visualizer simple modules. It
// subscribes on the obstacle loss module for physical object penetrated signals.
// The actual visualization of obstacle loss is done in derived modules.
//
// @see ~TracingObstacleLossCanvasVisualizer, ~TracingObstacleLossOsgVisualizer, ~ITracingObstacleLossVisualizer, ~TracingObstacleLossVisualizerBase
//
simple TracingObstacleLossVisualizerBase extends VisualizerBase
{
    parameters:
        string subscriptionModule = default(""); // module path where the visualizer subscribes for obstacle loss signals, root module by default
        bool displayIntersectionTrail = default(false); // display instersections of transmissions and physical objects, disabled by default
        bool displayFaceNormalVectorTrail = default(false); // display face normal vectors of instersections, disabled by default
        @class(TracingObstacleLossVisualizerBase);
}

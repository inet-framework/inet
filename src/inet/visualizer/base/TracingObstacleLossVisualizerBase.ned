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
// @see ~TracingObstacleLossCanvasVisualizer, ~TracingObstacleLossOsgVisualizer, ~ITracingObstacleLossVisualizer, ~VisualizerBase
//
simple TracingObstacleLossVisualizerBase extends VisualizerBase
{
    parameters:
        bool displayIntersections = default(false); // display instersections of transmissions and physical objects, disabled by default
        string intersectionLineColor = default("red");
        string intersectionLineStyle = default("solid");
        double intersectionLineWidth = default(1);

        bool displayFaceNormalVectors = default(false); // display face normal vectors of instersections, disabled by default
        string faceNormalLineColor = default("grey");
        string faceNormalLineStyle = default("solid");
        double faceNormalLineWidth = default(1);

        string fadeOutMode @enum("realTime", "animationTime", "simulationTime") = default("realTime"); // specifies how inactive obstacle losses fade out
        double fadeOutTime @unit(s) = default(1s); // how quickly link breaks fade away, 1 second by default
        double fadeOutAnimationSpeed = default(0); // animation speed while there are obstacle losses, value must be in the range (0, +inf)

        @class(TracingObstacleLossVisualizerBase);
}


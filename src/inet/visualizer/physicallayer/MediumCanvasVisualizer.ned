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
        double zIndex = default(0); // determines the drawing order of figures relative to other visualizers

        string signalShape @enum("ring", "sphere") = default("ring"); // shape for displaying signals
        double signalOpacity = default(0.5); // signal opacity, value must be in the range [0, 1]
        int signalRingCount = default(20); // number of stationary rings around the transmitter, value must be in the range [0, +inf)
        double signalRingSize @unit(m) = default(50m); // width of stationary rings, value must be in the range (0, +inf)
        double signalFadingDistance @unit(m) = default(50m); // distance parameter for exponential opacity decreasing, value must be in the range (0, +inf)
        double signalFadingFactor = default(1.2); // factor parameter for exponential opacity decreasing, value must be in the range (1, +inf)
        int signalWaveCount = default(20); // number of moving circles representing waves around the transmitter, value must be in the range [0, +inf)
        double signalWaveLength @unit(m) = default(50m); // distance between moving circles representing waves, value must be in the range (0, +inf)
        double signalWaveWidth @unit(m) = default(10m); // width of moving circles representing waves, value must be in the range (0, +inf)
        double signalWaveFadingAnimationSpeedFactor = default(1); // value must be in the range [0, 1]

        bool displayCommunicationHeat = default(false); // when enabled the radio medium displays recent successful receptions as a heat map overlay

        @class(MediumCanvasVisualizer);
}


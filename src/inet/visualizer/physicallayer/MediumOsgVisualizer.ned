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
// This module visualizes a medium using a 3D osg scene. It displays
// communication ranges and interference ranges as circles around network nodes.
// It displays an image at network nodes which are currently transmitting, and
// a different image at other network nodes whcih are currently receiving. It
// also displays radio signals as a growing 2D ring or a 3D sphere as they
// propagate through the medium. All 2D shapes such as circles, rings, and
// images can be displayed in 4 different planes: camera, xy, xz and yz. It's
// also possible to use animated images.
//
// @see ~MediumOsgVisualizer, ~MediumVisualizer, ~MediumVisualizerBase, ~IMediumVisualizer
//
simple MediumOsgVisualizer extends MediumVisualizerBase like IMediumVisualizer
{
    parameters:
        string signalShape @enum("ring", "sphere", "both") = default("ring"); // shape for displaying signals
        string signalPlane @enum("camera","xy","xz","yz") = default("xy"); // plane for 2 dimensional signal shapes
        double signalFadingDistance @unit(m) = default(100m); // distance parameter for exponential opacity decreasing
        double signalFadingFactor = default(1.5); // factor parameter for exponential opacity decreasing, value must be in the range (1.0, +inf)
        double signalWaveLength @unit(m) = default(100m); // distance between signal waves, value must be in the range (0, +inf)
        double signalWaveAmplitude = default(0.5); // relative opacity amplitude of signal waves, value must be in the range [0, 1]
        double signalWaveFadingAnimationSpeedFactor = default(1.0); // value must be in the range [0, 1]

        string transmissionPlane @enum("camera","xy","xz","yz") = default("camera"); // plane for transmission image
        string receptionPlane @enum("camera","xy","xz","yz") = default("camera"); // plane for reception image

        string communicationRangePlane @enum("camera","xy","xz","yz") = default("xy"); // plane for the transmission range circle
        string interferenceRangePlane @enum("camera","xy","xz","yz") = default("xy"); // plane for the interference range circle

        @class(MediumOsgVisualizer);
}


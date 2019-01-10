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
// This is a base module for radio visualizer simple modules.
//
// @see ~RadioCanvasVisualizer, ~RadioOsgVisualizer, ~IRadioVisualizer, ~VisualizerBase
//
simple RadioVisualizerBase extends VisualizerBase
{
    parameters:
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // module path to the network node visualizer module, refers to a sibling submodule by default

        bool displayRadios = default(false);
        bool displayRadioMode = default(true);
        bool displayReceptionState = default(true);
        bool displayTransmissionState = default(true);
        bool displayAntennaLobes = default(false);

        string radioFilter = default("**"); // which radios are considered, matches all radios by default

        string radioModeImages = default("block/process_vs block/timer_vs block/wrx_vs block/wtx_vs block/wrxtx_vs block/cogwheel_vs");
        string receptionStateImages = default("- - misc/receiving misc/receiving");
        string transmissionStateImages = default("- - misc/transmitting");
        double width = default(16);
        double height = default(16);
        string placementHint = default("bottomRight"); // annotation placement hint, space separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double placementPriority = default(0); // determines the order of annotation positioning

		bool antennaLobeNormalize = default(false); // true means maximum gain, false means 0dB is displayed at the given antennaLobeRadius
		bool antennaLobeRelativeLabels = default(false); // determines whether gain labels are relative or absolute (if true, maximum gain label is 0dB)
        bool antennaLobePlaneGlobal = default(true); // compute antenna lobe relative to the global or antenna local coordinate systems, global by default
        string antennaLobePlane @enum("view", "xy", "xz", "yz") = default("view"); // compute antenna lobe in this plane, view by default
        string antennaLobeMode @enum("logarithmic", "linear") = default("logarithmic");
        double antennaLobeLogarithmicBase = default(10);
        double antennaLobeLogarithmicScale = default(100); // distance change in pixels for 10dB
        double antennaLobeRadius = default(100); // distance in pixels for 0dB (1 gain) or maximum gain depending on antennaLobeNormalize parameter
        double antennaLobeStep @unit(deg) = default(10deg);
        double antennaLobeOpacity = default(0.25); // opacity of the antenna lobe
        bool antennaLobeLineSmooth = default(true);
        string antennaLobeLineColor = default("black"); // antenna lobe line color, black by default
        string antennaLobeLineStyle = default("solid"); // antenna lobe line style (solid, dashed, dotted)
        double antennaLobeLineWidth = default(1); // antenna lobe line width
        string antennaLobeFillColor = default("white"); // antenna lobe fill color, white by default

        @class(RadioVisualizerBase);
}


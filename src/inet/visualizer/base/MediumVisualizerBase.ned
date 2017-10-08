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
// This is a base module for radio medium visualizer simple modules. It keeps
// track of ongoing transmissions, ongoing receptions, and propagating signals.
//
// It subscribes for radioAdded, radioRemoved, transmissionAdded, transmissionRemoved,
// transmissionStarted, transmissionEnded, receptionStarted, receptionEnded
// signals at the module determined by the mediumModule parameter. The actual
// visualization of the radio medium is done in derived modules.
//
// @see ~MediumCanvasVisualizer, ~MediumOsgVisualizer, ~IMediumVisualizer, ~VisualizerBase
//
simple MediumVisualizerBase extends VisualizerBase
{
    parameters:
        string mediumModule = default("radioMedium"); // the medium module that is displayed, refers to a top level submodule by default
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // the network node visualizer module, refers to a sibling submodule by default
        
        string nodeFilter = default("*"); // which nodes are considered, matches all nodes by default
        string interfaceFilter = default("*"); // which interfaces are considered, matches all interfaces by default
        string packetFilter = default("*"); // which packets are considered, matches all packets by default

        bool displaySignals = default(false); // display signals propagating through the medium, disabled by default
        string signalColor @enum("light", "dark") = default("dark"); // signal color is a list of colors, a set of dark colors by default
        double signalPropagationAnimationSpeed = default(0/0); // animation speed while the very beginning or end of a signal is propagating on the medium, value must be in the range (0, +inf) or NaN, the latter means automatic setting
        double signalPropagationAnimationTime @unit(s) = default(1s); // signal propagation animation duration when signalPropagationAnimationSpeed is unspecified
        double signalPropagationAdditionalTime @unit(s) = default(0s); // additional simulation time to animate signal propagation after leaving last receiver
        double signalTransmissionAnimationSpeed = default(0/0); // animation speed while a signal is being transmitted on the medium, value must be in the range (0, +inf) or NaN, the latter means automatic setting
        double signalTransmissionAnimationTime @unit(s) = default(1s); // signal transmission animation duration when signalTransmissionAnimationSpeed is unspecified
        double signalAnimationSpeedChangeTime @unit(s) = default(1s); // animation speed changes smoothly from propagationAnimationSpeed to transmissionAnimationSpeed in the specified real time

        bool displayTransmissions = default(false); // display an image where a transmission is currently in progress, disabled by default
        string transmissionImage = default("misc/transmission.png"); // transmission image for ongoing transmissions
        string transmissionPlacementHint = default("topCenter"); // annotation placement hint, space separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double transmissionPlacementPriority = default(1); // determines the order of annotation positioning

        bool displayReceptions = default(false); // display an image where a reception is currently in progress, disabled by default
        string receptionImage = default("misc/reception.png"); // reception image for ongoing receptions
        string receptionPlacementHint = default("topCenter"); // annotation placement hint, space separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc. 
        double receptionPlacementPriority = default(-1); // determines the order of annotation positioning

        bool displayCommunicationRanges = default(false); // display communication ranges as circles around radios, enabled by default
        string communicationRangeLineColor = default("blue"); // communication range circle line color, blue by default
        string communicationRangeLineStyle = default("solid"); // communication range circle line style, solid by default
        double communicationRangeLineWidth = default(1); // communication range circle line width, 1 by default

        bool displayInterferenceRanges = default(false); // display interference ranges as circles around radios, disabled by default
        string interferenceRangeLineColor = default("gray"); // interference range circle color, gray by default
        string interferenceRangeLineStyle = default("solid"); // interference range circle line style, solid by default
        double interferenceRangeLineWidth = default(1); // interference range circle line width, 1 by default

        @class(MediumVisualizerBase);
}

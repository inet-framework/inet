//
// Copyright (C) 2013 OpenSim Ltd.
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
// It subscribes for notifications at the module determined by the mediumModule
// parameter. The actual visualization of the radio medium is done in derived
// modules.
//
// @see ~MediumCanvasVisualizer, ~MediumOsgVisualizer, ~IMediumVisualizer, and
// ~MediumVisualizerBase
//
simple MediumVisualizerBase extends VisualizerBase
{
    parameters:
        string mediumModule = default("radioMedium"); // module path of the medium module that is displayed, refers to a top level submodule by default

        bool displaySignals = default(false); // display signals propagating through the medium, disabled by default
        bool displayTransmissions = default(false); // display an image where a transmission is currently in progress, disabled by default
        bool displayReceptions = default(false); // display an image where a reception is currently in progress, disabled by default
        bool displayInterferenceRanges = default(false); // display interference ranges as gray circles around radios, disabled by default
        bool displayCommunicationRanges = default(false); // display communication ranges as blue circles around radios, enabled by default
        bool displayCommunicationTrail = default(false); // display recent successful receptions, disabled by default

        string transmissionImage = default(""); // transmission image for ongoing transmissions, no image by default
        string receptionImage = default(""); // reception image for ongoing receptions, no image by default

        double signalPropagationUpdateInterval @unit(s) = default(0s); // time interval between updates when there are propagating signals on the medium, no updates by default
        @class(MediumVisualizerBase);
}

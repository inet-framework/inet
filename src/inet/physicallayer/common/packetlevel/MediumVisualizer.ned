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

package inet.physicallayer.common.packetlevel;

import inet.physicallayer.contract.packetlevel.IMediumVisualizer;

simple MediumVisualizer like IMediumVisualizer
{
    parameters:
        bool displayCommunication = default(false);         // when enabled the radio medium displays the ongoing transmissions
        bool drawCommunication2D = default(false);          // when enabled the raido medium displays the ongoing transmissions as 2 dimensional rings instead of 3 dimensional spheres
        bool leaveCommunicationTrail = default(false);      // when enabled the radio medium displays recent successful receptions as straight blue lines from the transmission to the reception
        bool leaveCommunicationHeat = default(false);       // when enabled the radio medium displays recent successful receptions as a heat map overlay
        double updateCanvasInterval @unit(s) = default(0s); // the time interval between canvas updates when there are ongoing transmissions
        @display("i=block/app");
}

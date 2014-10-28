//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.mobility.common;

//
// Abstract base module for mobility models.
//
// @author Andras Varga
//
simple MovingMobilityBase extends MobilityBase
{
    parameters:
        double updateInterval @unit(s) = default(0.1s); // the simulation time interval used to regularly signal mobility state changes and update the display
}

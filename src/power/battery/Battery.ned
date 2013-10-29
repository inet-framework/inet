//
// Copyright (C) 2013 OpenSim Ltd.
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

package inet.power.battery;

import inet.power.common.PowerSourceBase;

//
// This module implements a voltage regulated battery.
//
// @author Levente Meszaros
//
simple Battery extends PowerSourceBase
{
	bool crashNodeWhenDepleted = default(true);
    double nominalCapacity @unit(J);
    double nominalVoltage @unit(V);
	double internalResistance = default(0); // @unit(Ω) = default(0Ω);
	
	@class(Battery);
}

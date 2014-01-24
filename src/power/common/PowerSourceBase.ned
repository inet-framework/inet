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

package inet.power.common;

import inet.power.contract.IPowerSource;

//
// This is an abstract base module for power source models.
//
// @author Levente Meszaros
//
simple PowerSourceBase like IPowerSource
{
    parameters:
        @display("i=block/cogwheel_s");
        @signal[powerConsumptionChanged];
        @statistic[residualCapacity](title="Residual capacity"; source=powerConsumptionChanged; record=vector; interpolationmode=linear);
}

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

package inet.power.base;

import inet.power.contract.IPowerAccumulator;
import inet.power.contract.IPowerSink;
import inet.power.contract.IPowerSource;

//
// This is an abstract base module for accumulator models.
//
// @author Levente Meszaros
//
simple PowerAccumulatorBase like IPowerSource, IPowerSink, IPowerAccumulator
{
    parameters:
        @display("i=block/plug");
        @signal[powerConsumptionChanged];
        @signal[powerGenerationChanged];
        @signal[residualCapacityChanged];
        @statistic[powerConsumption](title="Power consumption"; source=powerConsumptionChanged; record=vector; interpolationmode=sample-hold);
        @statistic[powerGeneration](title="Power generation"; source=powerGenerationChanged; record=vector; interpolationmode=sample-hold);
        @statistic[residualCapacity](title="Residual capacity"; source=residualCapacityChanged; record=vector; interpolationmode=linear);
}

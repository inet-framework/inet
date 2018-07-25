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

package inet.power.base;

import inet.power.contract.ICcEnergyStorage;

//
// This is an abstract base module for current based energy storage models.
// It defines shared signals and statistics.
//
// @see ~CcEnergyConsumerBase, ~CcEnergySourceBase, ~CcEnergyGeneratorBase, ~CcEnergySinkBase  
// @author Levente Meszaros
//
simple CcEnergyStorageBase like ICcEnergyStorage
{
    parameters:
        @display("i=block/plug");
        @signal[currentConsumptionChanged];
        @signal[currentGenerationChanged];
        @signal[residualChargeCapacityChanged];
        @statistic[currentConsumption](title="Current consumption"; source=currentConsumptionChanged; record=vector; interpolationmode=sample-hold);
        @statistic[currentGeneration](title="Current generation"; source=currentGenerationChanged; record=vector; interpolationmode=sample-hold);
        @statistic[residualChargeCapacity](title="Residual charge capacity"; source=residualChargeCapacityChanged; record=vector; interpolationmode=linear);
}


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

import inet.power.contract.IEpEnergyConsumer;

//
// This is an abstract base module for power based energy consumer models.
// It defines shared signals and statistics.
//
// @see ~EpEnergySourceBase, ~EpEnergyGeneratorBase, ~EpEnergySinkBase, ~EpEnergyStorageBase  
// @author Levente Meszaros
//
simple EpEnergyConsumerBase like IEpEnergyConsumer
{
    parameters:
        @display("i=block/plug");
        @signal[powerConsumptionChanged](type=double);
        @statistic[powerConsumption](title="Power consumption"; source=powerConsumptionChanged; record=vector; interpolationmode=sample-hold);
}


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

import inet.power.contract.IEpEnergySink;

//
// This is an abstract base module for power based energy sink models.
// It defines signals and statistics.
//
// @see ~EpEnergyConsumerBase, ~EpEnergySourceBase, ~EpEnergyGeneratorBase, ~EpEnergyStorageBase  
// @author Levente Meszaros
//
simple EpEnergySinkBase like IEpEnergySink
{
    parameters:
        @display("i=block/plug");
        @signal[powerGenerationChanged];
        @statistic[powerGeneration](title="Power generation"; source=powerGenerationChanged; record=vector; interpolationmode=sample-hold);
}


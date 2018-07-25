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

import inet.power.contract.ICcEnergySink;

//
// This is an abstract base module for current based energy sink models.
// It defines signals and statistics.
//
// @see ~CcEnergyConsumerBase, ~CcEnergySourceBase, ~CcEnergyGeneratorBase, ~CcEnergyStorageBase  
// @author Levente Meszaros
//
simple CcEnergySinkBase like ICcEnergySink
{
    parameters:
        @display("i=block/plug");
        @signal[currentGenerationChanged];
        @statistic[currentGeneration](title="Current generation"; source=currentGenerationChanged; record=vector; interpolationmode=sample-hold);
}


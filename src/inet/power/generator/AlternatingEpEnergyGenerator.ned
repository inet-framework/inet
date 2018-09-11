//
// Copyright (C) OpenSim Ltd.
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

package inet.power.generator;

import inet.power.base.EpEnergyGeneratorBase;

//
// This energy generator model alternates between two modes called generation
// and sleep mode. In generation mode it generates a randomly selected constant
// power for a random time interval. In sleep mode it doesn't generate energy
// for another random time interval.
//
// @see ~AlternatingEpEnergyConsumer
// @author Levente Meszaros
//
simple AlternatingEpEnergyGenerator extends EpEnergyGeneratorBase
{
    parameters:
        string energySinkModule = default("^.energyStorage"); // module path of energy sink
        volatile double powerGeneration @unit(W); // amount of power generated in generation mode
        volatile double generationInterval @unit(s); // time interval for generation mode
        volatile double sleepInterval @unit(s); // time interval for sleep mode
        @class(AlternatingEpEnergyGenerator);
}


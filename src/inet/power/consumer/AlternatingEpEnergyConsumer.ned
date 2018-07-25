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

package inet.power.consumer;

import inet.power.base.EpEnergyConsumerBase;

//
// This energy consumer model alternates between two modes called consumption
// and sleep mode. In consumption mode it consumes a randomly selected constant
// power for a random time interval. In sleep mode it doesn't consume energy for
// another random time interval.
//
// @see ~AlternatingEpEnergyGenerator
// @author Levente Meszaros
//
simple AlternatingEpEnergyConsumer extends EpEnergyConsumerBase
{
    parameters:
        string energySourceModule = default("^.energyStorage"); // module path of energy source
        volatile double powerConsumption @unit(W); // amount of power consumed in consumption mode
        volatile double consumptionInterval @unit(s); // time interval for consumption mode
        volatile double sleepInterval @unit(s); // time interval for sleep mode
        @class(AlternatingEpEnergyConsumer);
}


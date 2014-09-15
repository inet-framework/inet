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

package inet.power.consumer;

import inet.power.base.PowerConsumerBase;

//
// This module implements a simple power consumer which consumes a randomly
// selected constant power for a random time interval.
//
// @author Levente Meszaros
//
simple SimplePowerConsumer extends PowerConsumerBase
{
    parameters:
        string sinkModule = default("^.battery");
        volatile double powerConsumption @unit(W);
        volatile double interval @unit(s);
        @class(SimplePowerConsumer);
}

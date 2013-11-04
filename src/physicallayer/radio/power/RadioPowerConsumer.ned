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

package inet.physicallayer.radio.power;

import inet.power.contract.IPowerConsumer;

//
// This module provides a radio power consumer model.
//
simple RadioPowerConsumer like IPowerConsumer
{
    parameters:
        double sleepModePowerConsumption @unit(mW) = default(0mW);
        double receiverModeFreeChannelPowerConsumption @unit(mW) = default(0mW);
        double receiverModeBusyChannelPowerConsumption @unit(mW) = default(0mW);
        double receiverModeReceivingPowerConsumption @unit(mW) = default(10mW);
        double transmitterModeIdlePowerConsumption @unit(mW) = default(0mW);
        double transmitterModeTransmittingPowerConsumption @unit(mW) = default(100mW);

        @display("i=block/cogwheel_s");
}

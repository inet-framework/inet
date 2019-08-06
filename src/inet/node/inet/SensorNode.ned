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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.node.inet;

import inet.node.base.ApplicationLayerNodeBase;

//
// This module implements a wireless sensor node. It has one 802.15.4 wireless interface
// and an energy storage module by default. 
//# TODO and energyconsumer in the radio
//# TODO there is no management; the IdealEnergyStorage never depletes...
//
module SensorNode extends ApplicationLayerNodeBase
{
    parameters:
        @display("i=misc/sensor2");
        @figure[submodules];
        numWlanInterfaces = default(1);
        energyStorage.typename = default("IdealEpEnergyStorage");
        wlan[*].typename = default("Ieee802154NarrowbandInterface");
        wlan[*].radio.energyConsumer.typename = default("SensorStateBasedEpEnergyConsumer"); //FIXME does this consumer exist???
}

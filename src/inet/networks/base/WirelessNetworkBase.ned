//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.networks.base;

import inet.environment.contract.IPhysicalEnvironment;
import inet.physicallayer.wireless.common.contract.packetlevel.IRadioMedium;

network WirelessNetworkBase extends NetworkBase
{
    submodules:
        radioMedium: <default("RadioMedium")> like IRadioMedium if typename != "" {
            parameters:
                @display("p=100,400;is=s");
        }
        physicalEnvironment: <default("PhysicalEnvironment")> like IPhysicalEnvironment if typename != "" {
            parameters:
                @display("p=100,500;is=s");
        }
}
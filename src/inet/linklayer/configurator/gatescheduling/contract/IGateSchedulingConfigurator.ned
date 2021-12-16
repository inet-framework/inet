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

package inet.linklayer.configurator.gatescheduling.contract;

import inet.networklayer.configurator.contract.INetworkConfigurator;

//
// This module interface is implemented by gate scheduling configurator modules.
// A gate scheduling configurator is responsible for configuring all ~PeriodicGate
// modules of all traffic categories in all network interfaces in the network.
//
moduleinterface IGateSchedulingConfigurator extends INetworkConfigurator
{
    parameters:
        @display("i=block/cogwheel");
}


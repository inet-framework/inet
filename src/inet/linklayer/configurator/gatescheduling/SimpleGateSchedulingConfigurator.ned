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

package inet.linklayer.configurator.gatescheduling;

import inet.linklayer.configurator.gatescheduling.base.GateSchedulingConfiguratorBase;
import inet.linklayer.configurator.gatescheduling.contract.IGateSchedulingConfigurator;

//
// This module provides a gate scheduling algorithm that eagerly reserves time
// slots for the configured streams in the order of their priority (0 being the
// highest). The allocation makes sure that only one gate (traffic category) is
// open in all network interfaces at any given moment of time. This strategy
// may result in wasting to much time of the gate cycle and thus end up failing.
//
// This configurator is primarily useful to produce statistical results that can
// be used as a baseline in comparison with more advanced configurators.
//
simple SimpleGateSchedulingConfigurator extends GateSchedulingConfiguratorBase like IGateSchedulingConfigurator
{
    parameters:
        @class(SimpleGateSchedulingConfigurator);
}


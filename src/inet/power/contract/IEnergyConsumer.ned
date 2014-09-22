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

package inet.power.contract;

//
// The energy consumer models describe the energy consumption process of devices
// over time. For example, a radio consumes energy when it transmits or receives
// signals, or a CPU consumes energy when the network layer processes packets,
// or a display consumes energy when it's turned on, etc.
//
// @author Levente Meszaros
//
moduleinterface IEnergyConsumer
{
    parameters:
        @display("i=block/plug");
        @signal[powerConsumptionChanged](type=double);
}

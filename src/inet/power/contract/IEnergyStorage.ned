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

package inet.power.contract;

//
// The energy storage models describe devices that absorb energy produced by
// generators, and provide energy for consumers. For example, an electrochemical
// battery in a mobile phone provides energy for its display, its CPU, and its
// wireless communication device. It can also absorb energy produced by a solar
// panel installed on its display, or by a portable charger plugged into a wall
// socket.
//
// @see ~IEnergyConsumer, ~IEnergySource, ~IEnergyGenerator, ~IEnergySink, ~IEnergyManagement
// @author Levente Meszaros
//
moduleinterface IEnergyStorage extends IEnergySource, IEnergySink
{
    parameters:
        @display("i=block/plug");
}


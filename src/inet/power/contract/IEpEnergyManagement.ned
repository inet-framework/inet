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
// This interface extends the corresponding energy model interface. It requires
// implementations to describe energy consumption and energy generation with
// power [W] and storage capacity with energy [J]. The Ep is an abbreviation
// that is used for energy and power based interfaces.
//
// @see ~IEpEnergyConsumer, ~IEpEnergySource, ~IEpEnergyGenerator, ~IEpEnergySink, ~IEpEnergyStorage
// @author Levente Meszaros
//
moduleinterface IEpEnergyManagement extends IEnergyManagement
{
    parameters:
        @display("i=block/plug");
}


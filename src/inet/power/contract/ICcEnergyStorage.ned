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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

package inet.power.contract;

//
// This interface extends the corresponding energy model interface. It requires
// implementations to describe energy consumption and energy generation with
// current [A], and storage capacity with charge [C] and output voltage [V].
// The Cc is an abbreviation that is used for charge and current based interfaces.
//
// @see ~ICcEnergyConsumer, ~ICcEnergySource, ~ICcEnergyGenerator, ~ICcEnergySink, ~ICcEnergyManagement
//
moduleinterface ICcEnergyStorage extends ICcEnergySource, ICcEnergySink, IEnergyStorage
{
    parameters:
        @display("i=block/plug");
        @signal[residualChargeCapacityChanged](type=double);
}


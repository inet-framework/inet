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
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.icancloud.EnergySystem.EnergyMeter;

import inet.icancloud.EnergySystem.EnergyMeter.EnergyMeterUnit.IEnergyMeterUnit;
import inet.icancloud.EnergySystem.EnergyMeter.EnergyMeterController.EnergyMeterController;

module EnergyMeter
{
    parameters:

        bool activeEnergyMeter;

        string cpuMeterType = default("CPUMeter");
        string memoryMeterType = default("MemoryMeter");
        string storageMeterType = default("StorageMeter");
        string networkMeterType = default("NetworkMeter");

        @display("i=energy/laptop_battery");

    submodules:

        cpuMeter: <cpuMeterType> like IEnergyMeterUnit {
            @display("p=138,43");
        }
        storageMeter: <storageMeterType> like IEnergyMeterUnit {
            @display("p=50,43");
        }
        memoryMeter: <memoryMeterType> like IEnergyMeterUnit {
            @display("p=50,132");
        }
        networkMeter: <networkMeterType> like IEnergyMeterUnit {
            @display("p=144,132");
        }
        meterController: EnergyMeterController {

            @display("p=99,231");
        }
}

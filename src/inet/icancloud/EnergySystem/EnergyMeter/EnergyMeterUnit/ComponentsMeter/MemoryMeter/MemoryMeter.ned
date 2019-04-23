//
// Module that implements a memory meter. 
//
// This device is consists on:
//		- Memory meter core responsible for energy measurements 
//		- set of states representing each energy state. A state is defined by an id and
//        the energy consumed when device is performing operations at that state.
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2013-11-07
//


package inet.icancloud.EnergySystem.EnergyMeter.EnergyMeterUnit.ComponentsMeter.MemoryMeter;

import inet.icancloud.EnergySystem.EnergyMeter.EnergyMeterUnit.IEnergyMeterUnit;
import inet.icancloud.EnergySystem.EnergyMeter.EnergyMeterUnit.States.EState;
import inet.icancloud.EnergySystem.EnergyMeter.EnergyMeterUnit.ComponentsMeter.MemoryMeter.MemoryMeterCore.MemoryMeterCore;

module MemoryMeter like IEnergyMeterUnit
{
    parameters:
        int numEnergyStates;						// The number of different states
        string componentName;						// The name of the component
        string componentType = default("memory");  // The type of the component
        double consumptionBase;						// The consumption that a device have out of the component states
        int numModules;								// Number of memory modules plugged in the main board       						
        int numDRAMChips;							// Number of DRAM Chips per module
        @display("i=energy/battery_charged_25");

    submodules:
        state[numEnergyStates]: EState {
            @display("p=149,59");
        }

        core: MemoryMeterCore {
            parameters:
                componentName = componentName;
                componentType = componentType;
                numModules = numModules;
                numDRAMChips = numDRAMChips;

                @display("p=266,59");
        }

}

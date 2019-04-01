//
// Module that implements a cpu meter. 
//
// This device is consists on:
//		- CPU meter core responsible for energy measurements 
//		- set of states representing each energy state. A state is defined by an id and
//        the energy consumed when device is performing operations at that state.
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2013-11-07
//


package inet.icancloud.EnergySystem.EnergyMeter.EnergyMeterUnit.ComponentsMeter.CPUMeter;

import inet.icancloud.EnergySystem.EnergyMeter.EnergyMeterUnit.IEnergyMeterUnit;
import inet.icancloud.EnergySystem.EnergyMeter.EnergyMeterUnit.States.EState;
import inet.icancloud.EnergySystem.EnergyMeter.EnergyMeterUnit.ComponentsMeter.CPUMeter.CPUMeterCore.CPUMeterCore;

module CPUMeter like IEnergyMeterUnit
{
    parameters:
        int numEnergyStates;						// The number of different states
        string componentName;						// The name of the component
        string componentType = default("cpu");      // The type of the component
        double consumptionBase;						// The consumption that a device have out of the component states

        @display("i=energy/battery_charged_25");

    submodules:
        core: CPUMeterCore {
			componentType = componentType;
			componentName = componentName;
            @display("p=149,143");
        }
        state[numEnergyStates]: EState {
            @display("p=149,59");
        }
}

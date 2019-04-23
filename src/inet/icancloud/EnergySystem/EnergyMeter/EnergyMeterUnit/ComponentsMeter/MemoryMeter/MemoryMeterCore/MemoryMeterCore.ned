//
// Module that implements a memory meter core
//
// Memory meter core is responsible for performing energy measurements. 
// The core supply operations to get memory energy consumed (Joules) and memory power consumption(Watts)
// 
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2013-11-07
//

package inet.icancloud.EnergySystem.EnergyMeter.EnergyMeterUnit.ComponentsMeter.MemoryMeter.MemoryMeterCore;

simple MemoryMeterCore
{
    parameters:
            @class(icancloud::MemoryMeterCore);
       string componentName;
       string componentType;
       int numModules;								// Number of memory modules plugged in the main board       						
	   int numDRAMChips;							// Number of DRAM Chips per module
}

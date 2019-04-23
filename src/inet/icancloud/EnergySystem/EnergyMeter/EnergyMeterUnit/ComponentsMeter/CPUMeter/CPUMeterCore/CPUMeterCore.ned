//
// Module that implements a cpu meter core
//
// CPU meter core is responsible for performing energy measurements. 
// The core supply operations to get cpu energy consumed (Joules) and cpu power consumption(Watts)
// 
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2013-11-07
//

package inet.icancloud.EnergySystem.EnergyMeter.EnergyMeterUnit.ComponentsMeter.CPUMeter.CPUMeterCore;

simple CPUMeterCore
{
    @class(icancloud::CPUMeterCore);
    string componentName;
    string componentType;
}

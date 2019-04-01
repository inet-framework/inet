//
// Module that implements a storage meter core
//
// storage meter core is responsible for performing energy measurements.
// The core supply operations to get storage energy consumed (Joules) and storage power consumption(Watts)
// 
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2013-11-07
//
package inet.icancloud.EnergySystem.EnergyMeter.EnergyMeterUnit.ComponentsMeter.StorageMeter.StorageMeterCore;

simple StorageMeterCore
{
    @class(icancloud::StorageMeterCore);
    string componentType;
    string componentName;
}

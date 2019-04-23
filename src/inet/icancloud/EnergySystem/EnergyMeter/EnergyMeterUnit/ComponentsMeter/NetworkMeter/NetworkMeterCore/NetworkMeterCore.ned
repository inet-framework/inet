//
// Module that implements a network interface card (NIC) meter core
//
// NIC meter core is responsible for performing energy measurements.
// The core supply operations to get NIC energy consumed (Joules) and NIC power consumption(Watts)
// 
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2013-11-07
//

package inet.icancloud.EnergySystem.EnergyMeter.EnergyMeterUnit.ComponentsMeter.NetworkMeter.NetworkMeterCore;

simple NetworkMeterCore
{
    @class(icancloud::NetworkMeterCore);
    string componentType;
    string componentName;
}

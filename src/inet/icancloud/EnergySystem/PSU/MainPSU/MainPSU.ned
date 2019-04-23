//
// This module defines a generic PSU main parameters.
//
// The set of parameters are defined as quantity percents.
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2013-11-07
//

package inet.icancloud.EnergySystem.PSU.MainPSU;

import inet.icancloud.EnergySystem.PSU.IPSU;

simple MainPSU like IPSU
{
     @class(icancloud::MainPSU);
  	double wattage = default(1000.0);					// Rated output power in watts
  	double scale = default(1.0);						// Interval time to recalculate the energy loss by psu
    double psu_20 = default(81.46);						// Consumption in watts at 20 percent load
    double psu_50 = default(84.72);						// Consumption in watts at 50 percent load
    double psu_100 = default(81.46);					// Consumption in watts at 100 percent load  
    @display("i=energy/psu_30");
}

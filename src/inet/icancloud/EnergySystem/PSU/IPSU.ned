//
// This module interface defines PSU main parameters.
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2013-11-07
//
package inet.icancloud.EnergySystem.PSU;

moduleinterface IPSU{
    
    double wattage;					// Rated output power in watts
    double scale;					// Interval time to recalculate the energy loss by psu
        
}




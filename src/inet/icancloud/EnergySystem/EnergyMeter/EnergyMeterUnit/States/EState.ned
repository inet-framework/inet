//
// Module that implements an energy state. 
//
// A state is defined by an id and the energy consumed when device is performing operations at that state.
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2013-11-07
//

package inet.icancloud.EnergySystem.EnergyMeter.EnergyMeterUnit.States;

simple EState
{
     @class(icancloud::EState);
        string stateName;				// The name of the state
        double value;					// The consumption value associated to the stateName
}
//
// Module interface that defines a meter unit
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2013-11-07
//

package inet.icancloud.EnergySystem.EnergyMeter.EnergyMeterUnit;

moduleinterface IEnergyMeterUnit
{
    parameters:

        int numEnergyStates;								// The number of different states
        string componentName;								// The name of the component
        string componentType;								// The type of the component
        double consumptionBase;								// The consumption that a device have out of the component states
}

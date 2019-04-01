//
// This module is the software controller for the different cores at cpu module
//
// @author Gabriel González Castañé
// @date 2013-12-12

package inet.icancloud.Architecture.NodeComponents.Hardware.CPUs.CPUController;

simple CPUController

{
    parameters:
    @class(icancloud::CPUController);
        int numCPUs;								      
        bool indepentCores;
}

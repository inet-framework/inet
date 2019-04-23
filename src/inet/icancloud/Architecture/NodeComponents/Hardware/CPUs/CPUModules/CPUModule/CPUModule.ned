// Module that implements a Generic CPU Module.
//
// This module contains the following subModules:
//
// - N GenericProcessor
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2010-11-30
//
// @author Gabriel Gonz&aacute;lez Casta;&ntilde;&eacute
// @date 2013-10-27

package inet.icancloud.Architecture.NodeComponents.Hardware.CPUs.CPUModules.CPUModule;

import inet.icancloud.Architecture.NodeComponents.Hardware.CPUs.CPUController.CPUController;
import inet.icancloud.Architecture.NodeComponents.Hardware.CPUs.CPUcores.ICPUcore;
import inet.icancloud.Architecture.NodeComponents.Hardware.CPUs.ICPU_Module;


module CPU_Module like ICPU_Module
{
    parameters:
        int numCPUs;										// Number of CPU cores        
        string cpuCoreType = default("CPUCore");			// Type of each CPU core
        bool indepentCores = default(true);					// Are the CPU cores independent? 

        @display("i=device/cpu");
    gates:
        input fromOS[];		// Input gates from Operating System
        output toOS[];		// Output gates to Operating System


    submodules:
        CPUcore[numCPUs]: <cpuCoreType> like ICPUcore {

            @display("p=98,53");
        }

        eController: CPUController {
            parameters:
                numCPUs = numCPUs;
                indepentCores = indepentCores;
                @display("p=229,54");
        }

    connections allowunconnected:

        // Connections between all CPU cores and outside
        for i=0..numCPUs-1 {
            CPUcore[i].in <-- fromOS[i];
            CPUcore[i].out --> toOS[i];
        }
}

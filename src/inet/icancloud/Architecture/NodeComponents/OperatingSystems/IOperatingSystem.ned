// Module that represents a Generic Operating System Module.
//
// This module contains the following subModules:
//
// - A GenericNetworkService
// - A GenericCpuService
// - A GenericMemory
// - A GenericIORedirector
// - N GenericFSModule
// - A GenericVolumeManagerModule
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2010-11-30

package inet.icancloud.Architecture.NodeComponents.OperatingSystems;

moduleinterface IOperatingSystem

{
    parameters:
        string cpuSchedulerType;		// CPU Service type
        string vfsType;					// Virtual File System type		
        string fsModuleType;			// File System module type
        string vmModuleType;			// Volume Manager module type
        string syscallManagerType;		// Syscall Manager Type
        int numCPUs;					// Number of CPUs
        int numStorageSystems;		// Number of Storage Systems
		 

        @display("i=icons/OOSS");
    gates:
        input  fromApps[];			// Input gates from Applications
        input  fromStorageSystem[];	// Input gates from Storage Systems
        input  fromNet_TCP;			// Input gate from Network (Ethernet TCP)
        input  fromCPU[];			// Input gates from CPU

        output toApps[];			// Output gates to Applications
        output toStorageSystem[];	// Output gates to Storage Systems
        output toNet_TCP;			// Output gate to Network (Ethernet TCP)
        output toCPU[];				// Output gates to CPU
        input  fromInputMemory;   // Input gates from imputs
        input  fromOutputMemory;	// Input gate from output
        output toInputMemory;		// Output gates to inputs
        output toOutputMemory;		// Output gate to output
}

//
// This module defines a Virtual Machine Image. 
//
// A virtual machine image is defined to iCanCloud simulator as a type of machine without physical resources.
// The physical resources are managed by hypervisor and the virtual machine is linked to the hypervisor in order
// to perform those tasks.
// The main parameters that define a virtual machine are:
// 		- identification
//		- numCores
//		- memorySize_MB
//		- storageSize_GB
//
// This module is used to storage the parameters to be defined at omnetpp.ini defined by cloud provider to be rented by tenants
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2012-10-23
//

package inet.icancloud.Virtualization.VirtualMachines.SetDefinition;

import inet.icancloud.Applications.Apps.AppBase;

module VmImage
{
    parameters:
        string id;					// Identification
        int numCores;				// Number of cores
        int memorySize_MB @unit(MiB);		// Amount of memory (in MiB) of VM
        int storageSize_GB @unit(GiB);		// Amount of storage size (in GiB) of VM

}

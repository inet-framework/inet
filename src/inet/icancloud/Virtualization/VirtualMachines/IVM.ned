//
// This module interface defines the main parameters of a virtual machine
//
// A virtual machine is defined to iCanCloud simulator as a type of machine without physical resources.
// The physical resources are managed by hypervisor and the virtual machine is linked to the hypervisor in order
// to perform those tasks.
// The main parameters that define a virtual machine are:
//		- numCores
//		- memorySize_MB
//		- storageSize_GB
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2012-10-23
//

package inet.icancloud.Virtualization.VirtualMachines;

moduleinterface IVM
{
    parameters:
  
        int numCores;												
        int memorySize_MB @unit(MiB);
        int storageSize_GB  @unit(GiB);

}

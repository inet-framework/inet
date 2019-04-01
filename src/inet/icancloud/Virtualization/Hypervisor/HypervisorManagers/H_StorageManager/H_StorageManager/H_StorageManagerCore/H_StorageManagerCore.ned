//
// This module defines the core of the storage manager.
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2012-10-23
//

package inet.icancloud.Virtualization.Hypervisor.HypervisorManagers.H_StorageManager.H_StorageManager.H_StorageManagerCore;

simple H_StorageManagerCore
{
    parameters:
    @class(icancloud::H_StorageManagerCore);
        int numStorageServers;										// Number of Block Servers

        // Network file system parameters
        int requestSize_KB;		// Request size (in KB)

        //Parallel file system parameters
        int strideSize_KB;		// Stride Size (in KB)
		double ioOverhead;
		
    gates:
        input fromVMStorageServers[];
        output toVMStorageServers[];
        input fromNodeStorageServers[numStorageServers];
        output toNodeStorageServers[numStorageServers];
        input fromNET_Manager;
        output toNET_Manager;

        input from_storageCell[];
        output to_storageCell[];

}


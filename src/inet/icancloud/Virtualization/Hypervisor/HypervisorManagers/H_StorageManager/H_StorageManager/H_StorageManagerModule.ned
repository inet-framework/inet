//
// This module defines a storage manager module, one of the fourth main controllers of hypervisor.
//
// The module is composed by a storage cell controler and a set of storage cells. 
// The storage cells are created when each VM is linked.
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2012-10-23
//

package inet.icancloud.Virtualization.Hypervisor.HypervisorManagers.H_StorageManager.H_StorageManager;

import inet.icancloud.Virtualization.Hypervisor.HypervisorManagers.H_StorageManager.HW_Cells.IStorage_cell;
import inet.icancloud.Virtualization.Hypervisor.HypervisorManagers.H_StorageManager.IH_StorageManager;
import inet.icancloud.Virtualization.Hypervisor.HypervisorManagers.H_StorageManager.Schedulers.**.*;


module H_StorageManagerModule
{
    parameters:
		// Number of Storage Servers
        int numStorageServers;
        // Network file system parameters
        int requestSize_KB;		// Request size (in KB)
        //Parallel file system parameters
        int strideSize_KB;		// Stride Size (in KB)

        string storageCellType = "Storage_cell_basic";

        string storageManagerSched;
        double ioOverhead;
    gates:
        input fromVMStorageServers[];
        output toVMStorageServers[];
        input fromNodeStorageServers[numStorageServers];
        output toNodeStorageServers[numStorageServers];
        input fromNET_Manager;
        output toNET_Manager;

    submodules:

        storage_cell: <storageCellType> like IStorage_cell {
            parameters:

                // Network file system parameters
                requestSize_KB = requestSize_KB;
                // Parallel file system parameters
                strideSize_KB = strideSize_KB;

                @display("i=msg/job;p=80,160,row");
            gates:
                from_H_StorageManager;
                to_H_StorageManager;
        }

        storageManager: <storageManagerSched> like IH_StorageManager {
            parameters:
                numStorageServers = numStorageServers;
                storageCellType = storageCellType;
                // Storage remote parameters                        
                requestSize_KB = requestSize_KB;
                strideSize_KB = strideSize_KB;
                ioOverhead = ioOverhead;
                @display("i=icons/Redirector;p=80,67");
            gates:
                fromVMStorageServers[];
                toVMStorageServers[];
                fromNodeStorageServers[numStorageServers];
                toNodeStorageServers[numStorageServers];
                fromNET_Manager;
                toNET_Manager;
                from_storageCell[];
                to_storageCell[];
        }

    connections allowunconnected:

        // Connections between VM Storage Servers and Node Storage Servers
        for i=0..numStorageServers-1 {
            storageManager.fromVMStorageServers++ <-- fromVMStorageServers++;
            storageManager.toVMStorageServers++ --> toVMStorageServers++;
            storageManager.fromNodeStorageServers[i] <-- fromNodeStorageServers[i];
            storageManager.toNodeStorageServers[i] --> toNodeStorageServers[i];
        }

        // Connections between Network Manager and Storage Server Manager
        toNET_Manager <-- storageManager.toNET_Manager;
        fromNET_Manager --> storageManager.fromNET_Manager;

}


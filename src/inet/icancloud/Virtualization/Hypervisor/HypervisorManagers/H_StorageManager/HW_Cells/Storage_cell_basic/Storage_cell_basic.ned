//
// Module interface that defines a generic storage cell
//
// A storage cell is responsible for the remote storage system.
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2012-10-23
//

package inet.icancloud.Virtualization.Hypervisor.HypervisorManagers.H_StorageManager.HW_Cells.Storage_cell_basic;

import inet.icancloud.Virtualization.Hypervisor.HypervisorManagers.H_StorageManager.HW_Cells.IStorage_cell;

simple Storage_cell_basic like IStorage_cell
{
    parameters:
        @class(icancloud::Storage_cell_basic);
        // Network file system parameters
        	int requestSize_KB;		// Request size (in KB)        
        
        // Parallel file system parameters
		 	int strideSize_KB;		// Stride Size (in KB)				
		
		// Storage size GB
			int storageSizeGB  = default(20);
			
		// Identification parameters
			int pId = default (0);	// process identification
			int uId = default (0);	// user Identification	
        @display("i=msg/job");
    gates:
        input from_H_StorageManager;			// Input gate from H_BSManager
        output to_H_StorageManager;			// Output gate to H_BSManager
}
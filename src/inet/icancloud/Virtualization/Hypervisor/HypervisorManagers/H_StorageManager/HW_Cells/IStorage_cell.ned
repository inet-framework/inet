//
// Module interface that defines an storage cell
//
// A storage cell is responsible for the remote storage system.
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2012-10-23
//
package inet.icancloud.Virtualization.Hypervisor.HypervisorManagers.H_StorageManager.HW_Cells;

moduleinterface IStorage_cell 
{
    parameters:
        
        // Network file system parameters
        	int requestSize_KB;		// Request size (in KB)        
        
        // Parallel file system parameters
		 	int strideSize_KB;		// Stride Size (in KB)
		 	
		// Storage size GB
			int storageSizeGB;	
			
		// Identification parameters
			int pId;	// process identification
			int uId;	// user Identification			
	
        @display("i=msg/job");
    gates:
        input from_H_StorageManager;			// Input gate from H_BSManager
        output to_H_StorageManager;			// Output gate to H_BSManager
}
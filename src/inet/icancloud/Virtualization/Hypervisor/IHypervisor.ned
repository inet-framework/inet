//
// Module interface to define a Hypevisor.
//
// Each VM is linked inside this module to each controller.
// The hypervisor has four main controllers. Each controller is responsible for a physical resource: CPU, memory, network and storage.
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2012-10-23
//

package inet.icancloud.Virtualization.Hypervisor;

moduleinterface IHypervisor
{
    parameters:
        int numStorageServers; 									// Number of physical Storage Servers	
        int numCPUs; 	 										// Number of physical CPUs         
        int memorySize_MB @unit(MiB);        							// Amount of physical memory (MiB)
		int blockSize_KB @unit(KiB);									// size of blocks (Kb)		
        int storageSize_GB  @unit(GiB);										// Amount of physical storage size (GiB)       
        int numNetworkInterfaces;								// Number of physical network interfaces 
        string ip;												// Node's IP
        int storageApp_ModuleIndex;								// Index of the storageApp
        int connectionTimeOut;									// Time limit from a message send by hypervisor until the reception of the response
        string networkServiceType ;								// Network Service type
        string routingTableModule;
    gates:
        //Storage server
        input fromVMStorageServers[];
        output toVMStorageServers[];
        input fromNodeStorageServers[numStorageServers];
        output toNodeStorageServers[numStorageServers];

        //cpu
        input fromVMCPU[];
        output toVMCPU[];
        input fromNodeCPU[numCPUs];
        output toNodeCPU[numCPUs];

        //memory
        input fromVMMemoryI[];
        input fromVMMemoryO[];
        output toVMMemoryI[];
        output toVMMemoryO[];

        input fromNodeMemoryI;
        input fromNodeMemoryO;
        output toNodeMemoryI;
        output toNodeMemoryO;

        input fromVMNet[];
        output toVMNet[];
        input  fromNodeNet;			// Input gate from Network (Ethernet TCP)
		output toNodeNet;			// Output gate to Network (Ethernet TCP)
    
}


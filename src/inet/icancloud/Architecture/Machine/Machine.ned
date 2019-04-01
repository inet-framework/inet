//
// Module that implements a Machinep.
//
// This class models the base of an execution unit. It can be a Node, VM, or any hardware
// set of components that can execute applications
//
// @author Gabriel González Castañé
// @date 2014-12-12

package inet.icancloud.Architecture.Machine;

moduleinterface Machine
{
    parameters:

        @display("i=old/telnet");
       		       
            string hostName;								// Machines's hostname       
       		string osModuleType;							// Type of Operative system
            string cpuModuleType;							// CPUModule type as CPU type
            int numCores;									// Number of CPU cores		
         	int numStorageSystems;							// Number of Block Servers     
            int memorySize_MB @unit(MiB);							// Total amount of memory of this node (in MB)
    		int storageSize_GB @unit(GiB);								// Total amount of space for each storage system
            string ip;										// Node's IP
			bool IPForward;									// IP Forward        
            int namid;										// Nam ID	
            string routingFile;
}

//
// Module interface that represents a cloud scheduler
//
// @authors Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2014-12-11

package inet.icancloud.Management.CloudManagement.ResourcesProvisioning;

moduleinterface ICloudScheduler
{
   parameters:
        
		string virtualIPsBasis; 					// The virtual ip that the cloud will give to the VMs. It will be increased by 1 each new VM.					           
		double timeToStart; 						// The time that cloud waits until it starts
        bool liveMigration;							// Parameters to migrate VMs
        int dirtyingRate;							// The minimum quantity of data before disconnect the VM and reconnect it.
        int numberOfPFSRemoteServers;				// The number of parallel servers used as remote storage
        double timeBetweenScheduleEvents_s @unit(s);			// Time until the scheduler is activated again 
        double timeBetweenLogResults_s @unit(s);				// Time between all data center is checked extracting the energy measurements.
        bool printEnergyToFile;						// print the energy to a log file.     
        bool printEnergyTrace;						// print each schedule event the consumption values of the nodes    	
        bool printNodePowerConsumed;				// print Node instant consumption
    	bool printNodeEnergyConsumed;				// print Node energy consumed
    	bool printComponentsPowerConsumed;			// print main subsytems instant consumption
    	bool printComponentsEnergyConsumed;			// print main subsytems instant consumption     
    	bool printDataCenterPowerConsumed;			// print all the data-center energyConsumed
    	bool printDataCenterEnergyConsumed;			// print all the data-center instant consumption
        int remote_storage_VMs_per_node;			// THe maximum number of virtual machines allowed to access to a data server 
        double minimum_percent_storageNode_free;	// The percent minimum that represents the amount of storage in a data server before select other.
        int numberOfVMperNode;						// The maximum number of virtual machines into a node
        bool memorization;							// Parameter pending to test. Do not change the value
        bool outputCompression;					    // compress the output files
    	
}


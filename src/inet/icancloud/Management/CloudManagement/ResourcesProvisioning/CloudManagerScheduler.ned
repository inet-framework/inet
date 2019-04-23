//
// Module that is the core of a cloud manager scheduler
//
// The vmImages are the different virtual machines that the scheduler offers to the users
//
// @authors Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2014-12-11

package inet.icancloud.Management.CloudManagement.ResourcesProvisioning;

import inet.icancloud.Management.CloudManagement.ResourcesProvisioning.ICloudScheduler;
import inet.icancloud.Virtualization.VirtualMachines.IVM;

module CloudManagerScheduler like ICloudScheduler
{
    parameters:
        
        // The virtual ip that the cloud will give to the VMs. It will be increased by 1 each new VM.
		string virtualIPsBasis = default ("10.0.0.1");					      
     
     	// The time that cloud waits until it starts
		double timeToStart = default(0.0); // Time to start cloud manager
        
        // Parameters to migrate VMs
        bool liveMigration = default(false);
        
        // The minimum quantity of data before disconnect the VM and reconnect it.
        int dirtyingRate = default(256);
        
        // The number of parallel servers used as remote storage
        int numberOfPFSRemoteServers;
        
        // Time until the scheduler is activated again 
        double timeBetweenScheduleEvents_s @unit(s) = default(1s);
        
        // Time between all data center is checked extracting the energy measurements.
        double timeBetweenLogResults_s @unit(s) = default(1.0s);
        
        // print the energy to a log file.
        bool printEnergyToFile = default(true);     
        
        // print each schedule event the consumption values of the nodes
        bool printEnergyTrace = default(true);    	
        
    	// print Node instant consumption
    	bool printNodePowerConsumed = default (true);
    	
    	// print Node energy consumed
    	bool printNodeEnergyConsumed = default (true);
    	
    	// print main subsytems instant consumption
    	bool printComponentsPowerConsumed = default (true);
    	
    	// print main subsytems instant consumption
    	bool printComponentsEnergyConsumed = default (true);     
    	       	
    	// print all the data-center energyConsumed
    	bool printDataCenterPowerConsumed = default (true);
    	
    	// print all the data-center instant consumption
    	bool printDataCenterEnergyConsumed = default (true);

		// THe maximum number of virtual machines allowed to access to a data server 
        int remote_storage_VMs_per_node = default(10000);
        
        // The percent minimum that represents the amount of storage in a data server before select other.
        double minimum_percent_storageNode_free = default(0.5);
        
        // The maximum number of virtual machines into a node
        int numberOfVMperNode = default(10);
        
        // Parameter pending to test. Do not change the value
        bool memorization = default (false);
        
        // .ned Name for the VMs that will be the images that user virtual machines will replicate.
        string vmNedImageName = default ("BasicVM");
        
        // compress the output files
        bool outputCompression = default (false);
        
        @class(icancloud::AbstractCloudManager);
        @display("i=msg/job");

	submodules:
	    vmImage:<vmNedImageName> like IVM{
	        numCores = 0;
	        memorySize_MB = 0MiB;
	        storageSize_GB = 0GiB;
	        @display("i=old/telnet");
	    }
}

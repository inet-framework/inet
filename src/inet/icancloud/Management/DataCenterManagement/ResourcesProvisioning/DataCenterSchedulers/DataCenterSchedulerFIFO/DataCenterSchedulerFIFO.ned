// Module that implements a data center scheduler fifo.
//
//	It is under developement..
//
// @authors Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2014-06-10

package inet.icancloud.Management.DataCenterManagement.ResourcesProvisioning.DataCenterSchedulers.DataCenterSchedulerFIFO;

import inet.icancloud.Management.DataCenterManagement.ResourcesProvisioning.IDataCenterManager;


module DataCenterSchedulerFIFO like IDataCenterManager
{        
		double timeToStart; 										// The time that cloud waits until it starts
        int numberOfPFSRemoteServers;								// The number of parallel servers used as remote storage
        double timeBetweenScheduleEvents_s @unit(s) = default(1s);				// Time until the scheduler is activated again 
        double timeBetweenLogResults_s @unit(s) = default(1.0s);				// Time between all data center is checked extracting the energy measurements.
        bool printEnergyToFile = default(true);						// print the energy to a log file.     
        bool printEnergyTrace = default(true);						// print each schedule event the consumption values of the nodes    	
        bool printNodePowerConsumed = default (true);				// print Node instant consumption
    	bool printNodeEnergyConsumed = default (true);				// print Node energy consumed
    	bool printComponentsPowerConsumed = default (true);			// print main subsytems instant consumption
		bool printComponentsEnergyConsumed = default (true);		// print main subsytems instant consumption     
    	bool printDataCenterPowerConsumed = default (true);			// print all the data-center energyConsumed
    	bool printDataCenterEnergyConsumed = default (true);		// print all the data-center instant consumption
        double minimum_percent_storageNode_free= default(0.5);		// The percent minimum that represents the amount of storage in a data server before select other.
        bool memorization;											// Parameter pending to test. Do not change the value            	        
        bool outputCompression;    		    						// compress the output files
        @class(icancloud::DataCenterSchedulerFIFO);
        @display("i=msg/job");
        
}

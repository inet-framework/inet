//
// Module interface that represents a data center manager
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2014-12-11

package inet.icancloud.Management.DataCenterManagement.ResourcesProvisioning;

moduleinterface IDataCenterManager
{
   parameters:
       
		double timeToStart; 				 // Time to start the manager
        int numberOfPFSRemoteServers;		 // Number of parallel devices
        double timeBetweenScheduleEvents_s @unit(s);	 // Time between scheduling events
        double timeBetweenLogResults_s @unit(s);		 // Time between log energy results (in seconds)
        bool printEnergyToFile;              // true if the energy is printed to a file (to create it)
    	bool printEnergyTrace;     		     // true if the energy is printed as a predefined trace
    	bool memorization;					 // Parameter pending to test. Do not change the value
        bool outputCompression;    		     // compress the output files
}

        	
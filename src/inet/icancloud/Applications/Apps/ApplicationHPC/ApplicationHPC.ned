// Module that implements a GenericApp.
//
// This Application models a generic behavior of a HPC application
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2010-11-30
//
// @author updated to iCanCloud by Gabriel González Castañé
// @date 2012-05-17

package inet.icancloud.Applications.Apps.ApplicationHPC;

import inet.icancloud.Applications.Apps.IAPP;

simple ApplicationHPC like IAPP
{
	parameters:
	    @class(icancloud::ApplicationHPC);
		string application_netType = default("INET");				// Network type (INET or BASIC)        		
		bool allToAllConnections;				// Connect all processes between themselves?	
        int myRank = default(0);								// MPI process rank        
		double connectionDelay_s @unit(s);				// Delay time to establish connection with server!
		double startDelay_s @unit(s);					// Starting delay time!				
		
		int workersSet;							// Number of worker processes per set
		int numIterations;						// Number of iterations
		int sliceToWorkers_KB;					// Slice of data sent to each worker process (per iteration)
		int sliceToMaster_KB;					// Slice of data received for master process (per iteration)
		int sliceCPU;							// CPU processing (in MIPS) for each worker process (per iteration)
		
		bool workersRead;						// Worker processes read
		bool workersWrite;						// Worker processes write
			

    gates:
		input fromOS @loose;							// Input gate from OS (Operating System)
		output toOS @loose;							// Output gate to OS (Operating System)

}


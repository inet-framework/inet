// Module that implements a GenericApp.
//
// This Application models a generic behavior of a HPC application
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2010-11-30
//
// @author updated to iCanCloud by Gabriel González Castañé
// @date 2012-05-17

package inet.icancloud.Applications.Apps.ApplicationCheckpoint;

import inet.icancloud.Applications.Apps.IAPP;

simple ApplicationCheckpoint like IAPP

{
	parameters:
	    @class(icancloud::ApplicationCheckpoint);
	
		string application_netType;		// Network type (INET or BASIC)        		
		bool allToAllConnections;		// Connect all processes between themselves?	
        int myRank;						// MPI process rank        
		double connectionDelay_s @unit(s);		// Delay time to estalish connection with server!
		double startDelay_s @unit(s);			// Starting delay time!				
		
		int numIterations;				// Number of iterations		
		int dataToWrite_KB;				// Slice of data received for master process (per iteration)
		int sliceCPU;					// CPU processing (in MIPS) for each worker process (per iteration)

    gates:
		input fromOS @loose;					// Input gate from OS (Operating System)
		output toOS @loose;					// Output gate to OS (Operating System)

}


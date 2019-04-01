// Module that implements a GenericApp.
//
// This Application models a generic behavior of a HTC application
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2010-02-05
//
// @author updated to iCanCloud by Gabriel González Castañé
// @date 2012-05-17

package inet.icancloud.Applications.Apps.ApplicationHTC;

import inet.icancloud.Applications.Apps.IAPP;

simple ApplicationHTC like IAPP
{
	parameters:
	@class(icancloud::ApplicationHTC);
		string application_netType;		// Network type (INET or BASIC)	
        int myRank;				// MPI process rank		
		double startDelay_s @unit(s);		// Starting delay time!				
		
		int numProcessedFiles;	// Number of files to be processed
		int fileSize_MB;			// Size of each file processed
		int resultsSize_MB;		// Size of results in MB
		int fileNameSuffix;				// Suffix of the file name
		int cpuMIs;				// MIs needed for processing each file
			

    gates:
		input fromOS;			// Input gate from OS (Operating System)
		output toOS;			// Output gate to OS (Operating System)

}


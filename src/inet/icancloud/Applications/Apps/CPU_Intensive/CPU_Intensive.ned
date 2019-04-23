// Module that implements a CPU intensive app.
//
// This application generate requests to the cpu system of a set of MIs a set of iterations
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2010-11-30
//
// @author updated to iCanCloud by Gabriel González Castañé
// @date 2013-12-17

package inet.icancloud.Applications.Apps.CPU_Intensive;

import inet.icancloud.Applications.Apps.IAPP;

simple CPU_Intensive like IAPP

{
    parameters:
        @class(icancloud::CPU_Intensive);
        string application_netType = default ("INET");		// Network type (INET or BASIC)        
        double startDelay @unit(s);									// Starting delay time!        
        int inputSize;										// Input size of data
        int outputSize;										// Output size of data
        int MIs;											// Number of MIs to execute
        int iterations = default (0);						// Number of iterations
        @display("i=msg/job");
    gates:
        input fromOS @loose;			// Input gate from OS (Operating System)
        output toOS @loose;				// Output gate to OS (Operating System)
}



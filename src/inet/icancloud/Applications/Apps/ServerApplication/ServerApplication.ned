// Module that implements a Server application. 
//
// The parameters have to be used depending of the distribution. 
//
// @author Gabriel Gonz&aacute Casta;&ntilde;&eacute 
// @date 2012-12-01

package inet.icancloud.Applications.Apps.ServerApplication;

import inet.icancloud.Applications.Apps.IAPP;

simple ServerApplication like IAPP

{
    parameters:
        @class(icancloud::ServerApplication);
        string application_netType = default("INET");		// Network type (INET or BASIC)        
        double startDelay @unit(s);									// Starting delay time!        
        int inputSize = default(1246); 						// Input size of data_KB.- default value from http://www.webperformancetoday.com/2013/11/26/web-page-growth-151-percent/       
        int MIs;											// Number of MIs to execute
        
       	int hitsPerHour = default(540000);					// Number of hits per hour -> default = 150 x 3600
       	int uptimeLimit = default(0);						// Uptime limit for the serverApp. 0 = infinite

        @display("i=msg/job");
    gates:
        input fromOS @loose;								// Input gate from OS (Operating System)
        output toOS @loose;									// Output gate to OS (Operating System)
}



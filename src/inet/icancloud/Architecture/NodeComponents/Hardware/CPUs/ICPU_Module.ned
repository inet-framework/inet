// Module that represents a Generic CPU Module.
//
// This module contains the following subModules:
//
// - N GenericProcessor
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2010-11-30
//
// @author (modified) Gabriel Gonz&aacute;lez Casta;&ntilde;&eacute
// @date (modified) 2013-10-27

package inet.icancloud.Architecture.NodeComponents.Hardware.CPUs;

moduleinterface ICPU_Module

{
    parameters:
        
        int numCPUs;					// Number of CPU cores        
        string cpuCoreType;				// Type of each CPU core
        bool indepentCores; 	    	// True if management per core is independent (Ivy - Sandy Bridge).
        
        @display("i=device/cpu");
    gates:
        input fromOS[];					// Input gates from Operating System
        output toOS[];					// Output gates to Operating System

}



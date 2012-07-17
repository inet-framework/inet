//
// Copyright (C) 2012 Kyeong Soo (Joseph) Kim. All rights reserved.
//
// This software is written and distributed under the GNU General
// Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
// You must not remove this notice, or any other, from this software.
//


package inet.linklayer.hybridpon;

//
// Interface for Olt scheduler2 implementations.
// All Olt schedulers should implement this (e.g., OltSchedulerDRR).
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2012-07-09
//
moduleinterface IOltScheduler2
{
    parameters:
        @display("i=block/subqueue");        
//        double maxTxDelay @unit(s);	// max. limit to TX scheduling delay [sec]
//        double onuTimeout @unit(s);	// polling cycle from OLT to ONU [sec]        
    gates:
		inout ethg[];	// interface with Ethernet switch
		inout wdmg[];	// interface with WDM Layer
}

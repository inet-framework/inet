//
// Copyright (C) 2010 Kyeong Soo (Joseph) Kim. All rights reserved.
//
// This software is written and distributed under the GNU General
// Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
// You must not remove this notice, or any other, from this software.
//


package inet.linklayer.hybridpon;

//
// Interface for Olt scheduler implementations.
// All Olt schedulers should implement this (e.g., OltSchedulerSSSF).
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2010-02-25
//
moduleinterface IOltScheduler
{
    parameters:
        @display("i=block/subqueue");
        int cwMax @unit(bit);	// maximum grant to ONU [bit]        
        double maxTxDelay @unit(s);	// max. limit to TX scheduling delay [sec]
        double onuTimeout @unit(s);	// polling cycle from OLT to ONU [sec]        
    gates:
		inout ethg[];	// interface with Ethernet switch
		inout wdmg;	// interface with WDM Layer
}

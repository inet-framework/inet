///
/// @file   IOltScheduler.ned
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Feb/25/2010
///
/// @brief  Defines interfaces for a scheduler module in a hybrid TDM/WDM-PON OLT.
///
/// @remarks Copyright (C) 2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///

package inet.linklayer.hybridpon;


moduleinterface IOltScheduler
{
    parameters:
        @display("i=block/subqueue");
        int cwMax @unit(bit);	// maximum grant to ONU [bit]        
        double maxTxDelay @unit(s);	// max. limit to TX scheduling delay [sec]
        double onuTimeout @unit(s);	// polling cycle from OLT to ONU [sec]        
//        double queueSizePoll @unit(bit);	// size of FIFO queue for polling frames [bit]
//        string distances;			// string for ONU distances from OLT [km]

//        // Parameters specific to sequential scheduling
//        double queueSize @unit(bit);	// downstream queue memory size [bit]
//
//        // Parameters specific to improved sequential scheduling
//        double rsDepth;					// rescheduling depth
//
//        // Parameters specific to batch scheduling
//        double batchPeriod @unit(s);		// batch scheduling period [sec]
//        double voqSize @unit(bit);		// VOQ size [bit]
//        double voqThreshold;				// VOQ threshold (scaling factor) used in
//											// EdfWithLqf scheduler

    gates:
		inout ethg[];	// I/F with Ethernet switch
		inout wdmg[];	// I/F with WDM Optical Layer
}

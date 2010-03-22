///
/// @file   OltSchedulerSSSF.ned
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Feb/25/2010
///
/// @brief  Defines 'OltSchedulerSSSF' module in a hybrid TDM/WDM-PON OLT.
///
/// @remarks Copyright (C) 2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


package inet.linklayer.hybridpon;


simple OltSchedulerSSSF like IOltScheduler
{
    parameters:
        @display("i=block/join");
        int cwMax @unit(bit) = default(5e6b);		// maximum grant to ONU [bit] (default is 5 Mb)
        double maxTxDelay @unit(s) = default(2ms);	// max. limit to TX scheduling delay [sec]
        double onuTimeout @unit(s) = default(1ms);	// polling cycle from OLT to ONU [sec] (default is 1 ms)
        double queueSizePoll @unit(bit) = default(8388608b);	// size of FIFO queue for polling frames [bit] (default is 1 MB)
        double voqSize @unit(bit) = default(83886080b);			// VOQ size [bit] (default is 10 MB)
//        // Parameters specific to improved sequential scheduling
//        double rsDepth;					// rescheduling depth

    gates:
		inout ethg[];	// I/F with Ethernet switch
		inout wdmg;	// I/F with WDM Layer		
}

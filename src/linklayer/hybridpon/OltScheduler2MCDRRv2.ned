//
// Copyright (C) 2012 Kyeong Soo (Joseph) Kim. All rights reserved.
//
// This software is written and distributed under the GNU General
// Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
// You must not remove this notice, or any other, from this software.
//


package inet.linklayer.hybridpon;

//
// Implementation of "Sequential Scheduling with Schedule-time Framing (S<sup>3</sup>F)"
// algorithm reported in [1] for SUCCESS-HPON.
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2012-07-19
//
simple OltScheduler2MCDRRv2 like IOltScheduler2
{
    parameters:
        @display("i=block/join");
        double frameCapacity = default(100);	// per-subqueue capacity
        int quantum @unit(byte) = default(100B);	// quantum value in DRR scheduling
    gates:
		inout ethg[];	// interface with Ethernet switch
		inout wdmg[];	// interface with WDM Layer		
}

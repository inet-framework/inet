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
// This scheduler has the following features:
// <ul>
// <li>Based on VOQing (of Ethernet frames for downstream),
//     one FIFO queue per stream (either up- or downstream).
//     <ul>
//     <li>For the sake of implementation simplicity, now we have two
//         separate physical queues, VOQ and Tx Queue, but they are
//         handled as one logical queue, esp. in processing bit and
//         frame length counters;
//     </li>
//     </ul>
// </li>
// <li>Instant framing of downstream data as in ONU;
// </li>
// <li>Granting based on queue statuses for both up- and downstream
//     traffic with independent max. size limitation.
// </li>
// </ul>
//
// References:
// -# Kyeong Soo Kim, David Gutierrez, Fu-Tai An, and Leonid G. Kazovsky,
//    "Design and performance analysis of scheduling algorithms for WDM-PON
//    under SUCCESS-HPON architecture," IEEE/OSA Journal of Lightwave Technology,
//    vol. 23, no. 11, pp. 3716-3731, Nov. 2005.
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2012-07-09
//
simple OltScheduler2MCDRR like IOltScheduler2
{
    parameters:
        @display("i=block/join");
        double frameCapacity = default(100);	// per-subqueue capacity
        int quantum @unit(byte) = default(100B);	// quantum value in DRR scheduling
    gates:
		inout ethg[];	// interface with Ethernet switch
		inout wdmg[];	// interface with WDM Layer		
}

///
/// @file    DropTailTBFQueue.ned
/// @author  Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date    Tue Sep 27 11:55:00 2011
///
/// @brief   Defines WDM layer module in a hybrid TDM/WDM-PON ONU.
///
/// @remarks Copyright (C) 2011 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


package inet.networklayer.queue;

//
// Drop-tail queue with a token bucket filter (TBF), to be used in network
// interfaces requiring egree traffic shaping.
// Conforms to the OutputQueue interface and based on the Linux implementation
// of TBF with two token buckets, one for burst and mean rate control and
// (optionally) the other for MTU and peak rate control [1].
//
// References:
// -# Alexey Kuznetsov, token bucket filter queue implementation in Linux kernel
//    ("net/sched/sch_tbf.c").
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2011-09-27
//
simple DropTailTBFQueue like OutputQueue
{
    parameters:
        int frameCapacity = default(100);
		int burstSize @unit(byte) = default(1000B);
			// token bucket size; must be larger than mtu
			// note that with a finite line rate or peak rate,
			// the maximum size of burst is not equal to token bucket size.
		double meanRate @unit(bps) = default(10Mbps);	// token generation rate
		int mtu @unit(byte) = default(1500B);
		double peakRate @unit(bps) = default(100Mbps);	// must be larger than meanRate
        @display("i=block/queue");
    gates:
        input in;
        output out;
}




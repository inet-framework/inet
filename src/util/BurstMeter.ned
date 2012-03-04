///
/// @file   BurstMeter.ned
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Mar/3/2012
///
/// @brief  BurstMeter module for measuring frame/packet bursts
///
/// @remarks Copyright (C) 2012 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///

package inet.util;

//
// Frame/packet burst measurement module.
//
simple BurstMeter
{
    parameters:
        @node();
        @display("i=block/timer_vs");
        @signal[packetBurst](type="int");
		@statistic[packetBurst](title="packet burst"; record=stats,percentile?);
		int maxIFG = default(3);	// maximum number of IFGs allowed between two consecutive Etherent frames of a same burst
    gates:
        input in;
        output out;
}
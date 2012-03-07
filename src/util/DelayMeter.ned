///
/// @file   DelayMeter.ned
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Mar/1/2012
///
/// @brief  DelayMeter module for measuring frame/packet end-to-end delay.
///
/// @remarks Copyright (C) 2012 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///

package inet.util;

//
// End-to-end frame/packet delay measurement module.
//
simple DelayMeter
{
    parameters:
        @node();
        @display("i=block/timer_vs");
        @signal[packetDelay](type="double");
		@statistic[packetDelay](title="packet delay"; record=stats,vector?,percentile?);
    gates:
        input in;
        output out;
}



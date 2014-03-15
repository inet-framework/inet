///
/// @file   PerformanceMeter.ned
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   February/20/2014
///
/// @brief	PerformanceMeter module for measuring frame/packet end-to-end delay and
///			throughput.
///
/// @remarks Copyright (C) 2014 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///

package inet.util;

//
// Performance performance measurement utility module.
//
simple PerformanceMeter
{
    parameters:
        @node();
        @display("i=block/timer_vs");
        @signal[packetDelay](type="double");
        @signal[bitThruput](type="double");
        @signal[packetThruput](type="double");
        @statistic[packetDelay](title="packet delay"; record=stats,vector?,percentile?);
        @statistic[bitThruput](title="throughput (bit/s)"; record=stats,vector?,percentile?);
        @statistic[packetThruput](title="throughput (packet/s)"; record=stats,vector?,percentile?);
        double startTime @unit(s) = default(0s);
        double measurementInterval @unit(s) = default(1s);
    gates:
        input in;
        output out;
}

///
/// @file   LambdaSplitter.ned
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Mar/22/2010
///
/// @brief  Defines LambdaSplitter module, a logical wavelength router
///	on the path between an OLT and ONUs in a PON.
///
/// @remarks Copyright (C) 2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.

package inet.linklayer.hybridpon;


simple LambdaSplitter
{
    parameters:
        @node();
        @display("i=abstract/opticalswitch_s");
    gates:
        inout demuxg[];	// gates for WDM-demultiplexed optical streams (to ONUs)
        inout muxg[];	// a gate for WDM-multiplexed optical streams (to OLT)
}

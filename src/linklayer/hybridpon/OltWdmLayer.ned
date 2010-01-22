///
/// @file   WdmLayer.ned
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Thu Jan 21 15:43:39 2010
///
/// @brief  Defines interfaces for a WDM layer module.
///
/// @remarks Copyright (C) 2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


package inet.linklayer.hybridpon;


simple WdmLayer
{
    parameters:
		@node();
		@display("i=abstract/opticalswitch_s");
    gates:
        inout demuxg[];	// I/F for WDM-demultiplexed optical streams
        inout muxg;		// I/F for WDM-multiplexed optical streams
}

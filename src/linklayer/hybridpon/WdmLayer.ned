///
/// @file   WdmLayer.ned
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Thu Jul 9 14:02:00 2012
///
/// @brief  Defines WDM layer module in a hybrid TDM/WDM-PON ONU.
///
/// @remarks Copyright (C) 2012 Kyeong Soo (Joseph) Kim. All rights reserved.
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
		int ch = default(0);	// WDM channel index
    gates:
        inout pong;	// interface with PON layer
        inout phyg;	// interface with physical medium (i.e., optical fiber)
}

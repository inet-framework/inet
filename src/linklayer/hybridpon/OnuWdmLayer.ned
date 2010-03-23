///
/// @file   OnuWdmLayer.ned
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Thu Feb 22 10:10:00 2010
///
/// @brief  Defines WDM layer module in a hybrid TDM/WDM-PON ONU.
///
/// @remarks Copyright (C) 2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


package inet.linklayer.hybridpon;


simple OnuWdmLayer
{
    parameters:
		@node();
		@display("i=abstract/opticalswitch_s");
    gates:
        inout pong;	// interface with PON layer
        inout phyg;	// interface with physical medium (i.e., optical fiber)
}

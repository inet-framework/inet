///
/// @file   OnuMacLayer.ned
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Jul/06/2012
///
/// @brief  Defines MAC layer module in a hybrid TDM/WDM-PON ONU.
///
/// @remarks Copyright (C) 2012 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///

package inet.linklayer.hybridpon;


simple OnuMacLayer2
{
    parameters:
        @display("i=block/subqueue;p=92,124");
        int queueSize @unit(bit) = default(8e7b);	// size of FIFO queue (default is 80 Mb)
    gates:
        inout ethg;	// interface with Ethernet switch
        inout wdmg;	// interface with WDM Layer
}

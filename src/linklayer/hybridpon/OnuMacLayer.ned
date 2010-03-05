///
/// @file   OnuMacLayer.ned
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Feb/25/2010
///
/// @brief  Defines MAC layer module in a hybrid TDM/WDM-PON ONU.
///
/// @remarks Copyright (C) 2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///

package inet.linklayer.hybridpon;


simple OnuMacLayer
{
    parameters:
        @display("i=block/subqueue;p=92,124");
        int queueSize;        // size of FIFO queue
    gates:
        inout ethg;	// I/F with Ethernet switch
        inout wdmg;	// I/F with WDM Optical Layer
}

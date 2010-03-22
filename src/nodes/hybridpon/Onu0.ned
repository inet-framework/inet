///
/// @file   Onu.ned
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Feb/25/2010
///
/// @brief  Defines a hybrid TDM/WDM-PON ONU module.
///
/// @remarks Copyright (C) 2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


package inet.nodes.hybridpon;

import inet.nodes.ethernet.EtherSwitch;
import inet.linklayer.hybridpon.OnuMacLayer;
import inet.linklayer.hybridpon.OnuWdmLayer;

module Onu0
{
    parameters:
        @node();
        @labels(node,hybridpon-node);
        @display("i=device/smallrouter;bgb=188,228;bgp=2,2");
//        int queueSize @unit(bit) = default(8e7b); // size of FIFO queue in the Mac [bit]
    gates:
        inout ethg[] @labels(EtherFrame-conn);	// User Network I/F
        inout pong @labels(OpticalFrame-conn);	// I/F (i.e., with AWG)
    submodules:
        switch: EtherSwitch {
            parameters:
                @display("p=92,59");
			gates:
				ethg[sizeof(ethg)+1];
        }
        mac: OnuMacLayer {
            parameters:
                @display("p=92,124");
//                queueSize = queueSize;
        }
        wdm: OnuWdmLayer {
            parameters:
                @display("p=92,185");
        }
    connections:
        for i=0..sizeof(ethg)-1 {
            ethg++ <--> switch.ethg++;
        }
        switch.ethg++ <--> mac.ethg;
        mac.wdmg <--> wdm.demuxg;
        wdm.muxg <--> pong;
}

///
/// @file   Olt.ned
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Thu Jan 21, 2010
///
/// @brief  Defines a hybrid TDM/WDM-PON OLT module.
///
/// @remarks Copyright (C) 2009-2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


package inet.nodes.hybridpon;

import inet.linklayer.hybridpon.IScheduler;
import inet.linklayer.hybridpon.OltWdmLayer;
import inet.nodes.ethernet.EtherSwitch;


module Olt
{
    parameters:
        @display("i=device/router;bgb=177,239");
        int numOnus;	// number of ONUs connected to OLT
        string schType;	// type of 'Scheduler' (algorithm and operation mode)
    gates:
        inout pong;		// PON I/F (i.e., toward AWG)
        inout ethg;		// Network I/F (i.e., with metro/backbone Ethernet switch)
    //        output fromSchedulerToMonitor;
    submodules:
        switch: EtherSwitch {
            parameters:
                @display("p=90,48");
        }
        scheduler: <schType> like IScheduler {
            parameters:
                numOnus = numOnus;
                @display("p=90,118");
        }
        wdm: OltWdmLayer {
            parameters:
                @display("p=90,199");
        }
    connections:
        //        scheduler.toMonitor --> fromSchedulerToMonitor;
        ethg <--> switch.ethg[numOnus];
        for i=0..numOnus-1 {
            switch.ethg[i] <--> scheduler.ethg[i];
            scheduler.wdmg[i] <--> wdm.demuxg[i];
        }
        pong <--> wdm.muxg;
}
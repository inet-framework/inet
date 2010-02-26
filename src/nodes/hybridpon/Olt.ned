///
/// @file   Olt.ned
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Jan/21/2010
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

import inet.nodes.ethernet.EtherSwitch;
import inet.linklayer.hybridpon.IOltScheduler;
import inet.linklayer.hybridpon.OltWdmLayer;

module Olt
{
    parameters:
        @node();
        @display("i=device/router;bgb=177,239");
        int numOnus;	// number of ONUs connected to OLT
        int numReceivers;	// number of receivers
        int numTransmitters;	// number of transmitters
        string schType = default("OltSchedulerSSSF");	// type of 'Scheduler' (algorithm and operation mode)
    gates:
        inout ethg[];	// Service Network I/F (i.e., with metro/backbone)
        inout pong[];	// PON I/F (i.e., with AWG)
    //        output fromSchedulerToMonitor;
    submodules:
        switch: EtherSwitch {
            parameters:
                @display("p=90,48");
        }
        scheduler: <schType> like IOltScheduler {
            parameters:
                @display("p=90,118");
//                numOnus = numOnus;
        }
        wdm: OltWdmLayer {
            parameters:
                @display("p=90,199");
        }
    connections:
        //        scheduler.toMonitor --> fromSchedulerToMonitor;
        for i=0..sizeof(ethg)-1 {
            ethg++ <--> switch.ethg++;
        }
        for i=0..numOnus-1 {
            switch.ethg++ <--> scheduler.ethg++;
            scheduler.wdmg++ <--> wdm.demuxg++;
            wdm.muxg++ <--> pong++;
        }
}

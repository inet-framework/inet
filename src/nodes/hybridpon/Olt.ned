// $Id$
//------------------------------------------------------------------------------
//	Olt.ned --
//
//	This file defines a PON OLT module.
//
//  Copyright (C) 2009 Kyeong Soo (Joseph) Kim
//------------------------------------------------------------------------------


package inet.nodes.hybridpon;

import inet.linklayer.hybridpon.IScheduler;
import inet.nodes.ethernet.EtherSwitch;


module Olt
{
    parameters:
        @display("bgb=188,169;bgp=2,2");
        int numOnus;	// number of ONUs connected to OLT
        string schType;	// type of 'Scheduler' (algorithm and operation mode)
    gates:
		inout pong;		// PON I/F (i.e., toward AWG)
		inout ethg;		// Network I/F (i.e., with metro/backbone Ethernet switch)
//        output fromSchedulerToMonitor;
//        output fromIpPktGeneratorToMonitor;
//        output fromIpPktSinkToMonitor;
    submodules:
        scheduler: <schType> like IScheduler {
            parameters:
//                @display("i=block/classifier;p=94,116");
                @display("i=block/classifier");
                numOnus = numOnus;
        }
        switch: EtherSwitch {
            parameters:
//                @display("p=145,257,row,110;q=l2queue");
        }
    connections:
//        scheduler.toMonitor --> fromSchedulerToMonitor;
		pong <--> scheduler.pong;
		for i=0..numOnus-1 {
		    scheduler.ethg[i] <--> switch.ethg[i];
		}
		ethg <--> switch.ethg[numOnus];
}

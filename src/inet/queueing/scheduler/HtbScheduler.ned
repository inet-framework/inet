// Hierarchical Token Bucket Implementation for OMNeT++ & INET Framework
// Copyright (C) 2021 Marija Gajić (NTNU), Marcin Bosk (TUM), Susanna Schwarzmann (TU Berlin), Stanislav Lange (NTNU), and Thomas Zinner (NTNU)
// 
// SPDX-License-Identifier: GPL-2.0-or-later
//
//
//
// This implementation is heavily based on the implementation of Linux HTB qdisc by Martin Devera (https://github.com/torvalds/linux/blob/master/net/sched/sch_htb.c)
// Code base taken from the "PriorityScheduler"
//

package inet.queueing.scheduler;

import inet.queueing.base.PacketSchedulerBase;
import inet.queueing.contract.IPacketScheduler;

simple HtbScheduler extends PacketSchedulerBase like IPacketScheduler
{
    parameters:
        xml htbTreeConfig; // The htb tree structure

        bool htbHysterisis = default(false);
		
        int mtu @unit(B) = default(1500B);
        int phyLayerHeaderLength @unit(B) = default(7B); // Adjust the PHY layer header size depending on the PHY MAC used. 7 Bytes for PPP and 8 Bytes for Ethernet. Default 7B for PPP interfaces.
        
        bool checkHTBTreeValuesForCorectness = default(true); // Check if some important (burst, cburst, quantum) inputted values in the XML Tree are correct. Will throw errors on incorrect values!
        bool adjustHTBTreeValuesForCorectness = default(true); // Adjust some important (burst, cburst, quantum) inputted values in the XML Tree if they are incorrect. Will just adjust incorrect values if the above check is disabled

        @class(HtbScheduler);

        @signal[dequeueIndex](type=long);
        @statistic[dequeueIndex](title="Index of a queue that just dequeued"; record=vector);


        @signal[class-*-tokenLevel](type=long);
        @statisticTemplate[tokenLevel](record=vector);
        @signal[class-*-ctokenLevel](type=long);
        @statisticTemplate[ctokenLevel](record=vector);

        @signal[class-*-deficit*](type=long);
        @statisticTemplate[deficit](record=vector);

        @signal[class-*-mode](type=long);
        @statisticTemplate[classMode](record=vector);
}

// $Id$
//------------------------------------------------------------------------------
//	SSSF.ned --
//
//	This file defines "SSSF (Sequential Scheduling with Schedule-time Framing)"
//	scheduler module in a PON OLT.
//
//  Copyright (C) 2009 Kyeong Soo (Joseph) Kim
//------------------------------------------------------------------------------ 


package inet.linklayer.hybridpon;


simple SSSF like IScheduler
{
    parameters:
        @display("i=block/join");
        int cwMax @unit(bit);		// maximum grant to ONU [bit]
        int numOnus;				// number of ONUs/channels
        int numReceivers;			// number of receivers
        int numTransmitters;		// number of transmitters
        int numUsersPerOnu;			// number of users per ONU
        double maxTxDelay @unit(s);	// max. limit to TX scheduling delay [sec]
        double onuTimeout @unit(s);	// polling cycle from OLT to ONU [sec]
        double queueSizePoll @unit(bit);	// size of FIFO queue for polling frames [bit]
        double voqSize @unit(bit);			// VOQ size [bit]
        string distances;			// string for ONU distances from OLT [km]

//        // Parameters specific to improved sequential scheduling
//        double rsDepth;					// rescheduling depth

    gates:
        input in;					// to receive PON frames from ONUs
        input fromPacketGenerator;	// to receive generated IP packets
        output out;					// to send PON frames to ONUs
        output toMonitor;			// to monitor
        output toPacketSink;		// to sink received IP packets
}

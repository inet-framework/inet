// $Id$
//-------------------------------------------------------------------------------
//	IScheduler.ned --
//
//	This file defines interfaces for scheduler modules in a PON
//	OLT.
//
//	Copyright (C) 2009 Kyeong Soo (Joseph) Kim
//-------------------------------------------------------------------------------


package inet.linklayer.hybridpon;


moduleinterface IScheduler
{
    parameters:
        @display("i=block/join");
        int cwMax @unit(bit);		// maximum grant to ONU [bit]
        int numOnus;				// number of ONUs/channels
        int numReceivers;			// number of receivers
        int numTransmitters;		// number of transmitters
        int numUnisPerOnu;			// number of users per ONU
        double maxTxDelay @unit(s);	// max. limit to TX scheduling delay [sec]
        double onuTimeout @unit(s);	// polling cycle from OLT to ONU [sec]
//        double queueSizePoll @unit(bit);	// size of FIFO queue for polling frames [bit]
        string distances;			// string for ONU distances from OLT [km]

//        // Parameters specific to sequential scheduling
//        double queueSize @unit(bit);	// downstream queue memory size [bit]
//
//        // Parameters specific to improved sequential scheduling
//        double rsDepth;					// rescheduling depth
//
//        // Parameters specific to batch scheduling
//        double batchPeriod @unit(s);		// batch scheduling period [sec]
//        double voqSize @unit(bit);		// VOQ size [bit]
//        double voqThreshold;				// VOQ threshold (scaling factor) used in
//											// EdfWithLqf scheduler

    gates:
//        input in;					// to receive PON frames from ONUs
////        input fromPacketGenerator;	// to receive generated IP packets
//		input fromSwitch[];			// from Ethernet switch (bridge)
//        output out;					// to send PON frames to ONUs
////        output toMonitor;			// to monitor
////        output toPacketSink;		// to sink received IP packets
//		output toSwitch[];			// to Ethernet switch (bridge)
		inout pong;		// PON I/F
		inout ethg[];	// I/F with Ethernet switch
}

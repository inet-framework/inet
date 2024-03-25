// Copyright (C) 2024 Daniel Zeitler
// SPDX-License-Identifier: LGPL-3.0-or-later

//
// The Media Redundancy Protocol (MRP) is a network protocol similar to STP and RSTP that ensures a loop-free
// topology for an Ethernet local area network. It is defined in IEC 62439-2. The basic function of MRP
// is to prevent bridge loops by building up a logical line topology based on a physical ring topology.
// MRP nodes have to be connected directly to each other with a maximum of 50 nodes per ring defined in standard.
// In case of a network, link or node error, a node in manager role is able to change the topology and enable 
// his second ring port and reestablish a working network within a guaranteed upper time limit.
// A mrp node may be a switch or single device. Each node has exactly two ports which are part of the mrp ring, all 
// other ports act like normal switch ports.
// Ring health is checked by a periodic test frame sent by node in manager role. If this test frame is not received 
// within 5 test intervals, topology change is initiated. Ring failure may also be recognized by clients, which are 
// able to detect link error locally. In this case, client is sending a link change frame, upon which the manager may 
// react with a topology change. 
//
// MRP makes use of per-port configuration data defining role and state.
// They can be set using ~L2NetworkConfigurator. Port state is kept in the switch's
// ~InterfaceTable.
//
// @see ~L2NetworkConfigurator, ~InterfaceTable
//


package inet.linklayer.mrp.mediaredundancy;

import inet.linklayer.contract.IMediaRedundancy;

simple Mrp like IMediaRedundancy
{
    parameters:
        @class(Mrp);
        //switches do not detect a link change immediatly. this parameter sets mean time and deviation in milliseconds for
        //the simulated delay of a link change event
        volatile double linkDetectionDelay @unit(ms) = default(truncnormal(385ms,325ms));
        
        //switches take a certain amount of time for processing each received packet.
        //this parameter sets the mean processsing delay and deviation in microseconds
        //and is applied in handleMessageWhenUp()
        volatile double processingDelay @unit(us) = default(truncnormal(100us,80us));
        
        // MRP defines 4 Maximum Recovery Time settings for ring: 500 ms , 200 ms, 30 ms or 10 ms
        // this time provides a hard upper time limit guaranteed by 
        // Depending on this parameter the interval for test frames, topology and link change times are adapted
        // should be set to the same value on all nodes per ring
        int timingProfile @unit(ms) = default(500ms);
        
        // Each MRP node can either be disabled (value 0, no mrp function), MediaRedundancyClient (value 1, MRC)
        // MediaRedundancyManager (value 1, MRM) or MediaRedundancyAutoManager (value 2, MRA)
        // Each Network may have only 1 MRM, all other nodes have to be in MRC role
        // MRAs are able to vote which node shall take MRM role by priority and MAC address value, all other nodes 
        // dynamicly switch to MRC role. Do not mix MRM and MRA! although it is possible, either choose a specific node as explicit MRM
        // or set one or more nodes to MRA role
        //"DISABLED" == 0, "CLIENT" == 1, "MANAGER" == 2, "MANAGER_AUTO_COMP" == 3, "MANAGER_AUTO" == 4
        int mrpRole = default(0);
        
        //Each MRP node has exactly 2 ring interfaces, which have to be set by the network administrator
        //PortIndexNumber of ringport1
        int ringPort1 = default(0);
		//PortIndexNumber of ringport2
		int ringPort2 = default(1);
		
		//A mrp ring has a UUID, in 2 64-Bit parts
		//every node in a ring has to be set to the same UUID
		int uuid0 = default(33333);
		int uuid1 = default(33333);	
		
		//it is possible to enable an active link check on layer 2 for each ring port
		//link check is based on 802.1q-CFM
		//Keep the additional load on node and network in mind!
		bool enableLinkCheckOnRing = default(false);
		
		// CCM link check can be done every 3.3 ms or every 10 ms
        double ccmInterval @unit(ms) = default(10ms);
        	
		//mrp rings can be redundantly connected by interconnection nodes
        //in order to handle the necessary communication, normal mrp nodes
        //should be aware of the used check methods. Check can be done either by RingCheck (similar to ring test mechanism)
        //or by link check on layer 2 
		bool interconnectionRingCheckAware = default(true);
        bool interconnectionLinkCheckAware = default(true);
		
        // Shows the ring by coloring connections in the network graphics.
        // Label ethernet interface with port role and status
        bool visualize = default(true);
        
        // The path to the InterfaceTable module
        string interfaceTableModule;
        
        // The path to the MacForwardingTable module
        string macTableModule;
        
        // The path to the MrpRelay module
        string mrpRelayModule;
        
		//manager variables
		// A MRM or MRA can have higher or lower priority. 0x0000 is the highest possible value, 0xFFFF the lowest
		// MRM should be assigned with on of the following values: HIGHEST = 0x0000, HIGH = 0x4000, DEFAULT = 0x8000
		// If the node is a MRA, assign one of these: MRAHIGHEST = 0x9000, MRADEFAULT = 0xA000, MRALOWEST = 0xFFFF
		// Depending on Priority and Macaddress MRAs define the acting manager 
		int mrpPriority = default(0xA000);
		
		// a mrm may or may not support clients that are able to block their ring ports
		bool nonBlockingMRC = default(true);
		
		// defines, if manager reacts on link change frames sent by client or if manager relies only on ring tests
        bool reactOnLinkChange = default(true);
        
        //defines, if manager activly tests the ring, or only relies on the link change frames sent by client
        bool checkMediaRedundancy = default(true);
        
        //defines, if manager is able to set his own secondary ring port to FORWARDING and therefore change the ring topology
        bool noTopologyChange = default(false);
        
        //client variables
        // enables the client to set one of his ring ports to BLOCKED, if advised by manager via topology change frame
        bool blockedStateSupported = default(true);
        
        @display("i=block/network2");
        @signal[testSignal](type="long");
        @signal[topologyChangeSignal](type="unsigned long");
        @signal[linkChangeSignal](type="double");
        @signal[continuityCheckSignal](type="long");
        @signal[clearFDBSignal](type="double");
        @signal[receivedTestSignal](type="unsigned long");
        @signal[receivedChangeSignal](type="unsigned long");
        @signal[receivedContinuityCheckSignal](type="long");
        @signal[ringStateChangedSignal](type="unsigned long");
        @signal[portStateChangedSignal](type="unsigned long");
        @statistic[testFrameSignal](title="Testframes sent"; source="testSignal"; record=count,vector; interpolationmode=none);
        @statistic[linkChangeSignal](title="LinkChangeSignal in us"; source="linkChangeSignal"; record=count,vector; interpolationmode=none);
        @statistic[topologyChangeSignal](title="TopologyChangeSignal in us"; source="topologyChangeSignal"; record=count,vector; interpolationmode=none);
        @statistic[fDBCleared](title="FDB cleared in us"; source="clearFDBSignal"; record=count,vector; interpolationmode=none);
        @statistic[ringTime](title="T Ring in us"; source="receivedTestSignal"; record=vector,stats,histogram; interpolationmode=none);
        @statistic[timeToChange](title="Time till announced Change in ms"; source="receivedChangeSignal"; record=vector,stats,histogram; interpolationmode=none);
        @statistic[sentContinuityCheck](title="Sent Continuity Checks"; source="continuityCheckSignal"; record=count,vector; interpolationmode=none);
        @statistic[receivedContinuityCheck](title="Received Continuity Checks"; source="receivedContinuityCheckSignal"; record=count,vector; interpolationmode=none);
        @statistic[portStateChanged](title="Time of Port State Change in us"; source="portStateChangedSignal"; record=count,vector; interpolationmode=none);
        @statistic[ringStateChanged](title="Time of Ring State Change in us"; source="ringStateChangedSignal"; record=count,vector; interpolationmode=none);
    gates:
        input relayIn;
        output relayOut;
}

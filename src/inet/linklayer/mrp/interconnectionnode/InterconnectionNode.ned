// Copyright (C) 2024 Daniel Zeitler
// SPDX-License-Identifier: LGPL-3.0-or-later

//
// The Media Redundancy Protocol (MRP) is a network protocol similar to STP and RSTP that ensures a loop-free
// topology for an Ethernet local area network. It is defined in IEC 62439-2. The basic function of MRP
// is to prevent bridge loops by building up a logical line topology based on a physical ring topology.
// MRP nodes have to be connected directly to each other with a maximum of 50 nodes per ring defined in standard.
// In case of a network, link or node error, a node in manager role is able to change the topology and enable 
// his second ring port and reestablish a working network within a guaranteed upper time limit.
// A mrp node may be a switch or single device. 
//
// Interconnection nodes build up a redundant interconnection of two independent mrp rings by forming an interconnection ring
// consisting of exactly 4 interconnection nodes. One interconnection node has to be in role interconnection manager, the other three
// nodes are in role interconnection client. Two of these interconnection nodes are part of mrp ring 1, the other 2 nodes are 
// part of mrp ring 2. 
// Each interconnection node is also a normal mrp node and therefore has exactly two port which are part of the mrp ring.
// Additonally interconnection nodes have exactly one interconnection port. All other ports act like normal switch ports.
// Interconnection ring health is checked either by a periodic interconnection test frame sent by node in manager role (ring check)
// or by local, active link check on layer 2 of each interconnection node (link check).
//
// MRP makes use of per-port configuration data defining role and state.
// They can be set using ~L2NetworkConfigurator. Port state is kept in the switch's
// ~InterfaceTable.
//
// @see ~L2NetworkConfigurator, ~InterfaceTable
//

package inet.linklayer.mrp.interconnectionnode;

import inet.linklayer.contract.IMediaRedundancy;
import inet.linklayer.mrp.mediaredundancynode.MediaRedundancyNode;


simple InterconnectionNode like IMediaRedundancy
{
    parameters:        
        //each interconnection has an name assigned, for human readable identifikation
        object InterconnectionName = default("interconnection1");
        
        //switches do not detect a link change immediatly. this parameter sets mean time and deviation in milliseconds for
        //the simulated delay of a link change event
        object linkDetectionDelayMean = default(385);
        object linkDetectionDelayDev = default(325);
        
        //switches take a certain amount of time for processing each received packet.
        //this parameter sets the mean processsing delay and deviation in microseconds
        object processingDelayMean = default(100);
        object processingDelayDev = default(80);
        
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
        int expectedRoleByNum = default(0);
                
        // each interconnection node has to take over one of two possible roles regarding interconection: 
        // interconnection client or interconnection manager
        // only one of the needed four nodes may have manager role, the other three nodes shall be in client role 
        int inRole = default(1);        
        
        //Each MRP interconnection node has exactly 2 ring interfaces and 1 interconnectionPort, which have to be set by the network administrator
        //PortNumber of ringport1
        int ringPort1 = default(0);
		//PortNumber of ringport2
		int ringPort2 = default(1);
		//PortNumber of interconnectionPort
		int InterconnectionPort = default(2);
		
		//A mrp ring has a UUID, in 2 64-Bit parts
		//every node in a ring has to be set to the same UUID
		int uuid0 = default(33333);
		int uuid1 = default(33333);
		
        //each interconnection has an unique indentifier, assign a value from 0 - 65535
        int interConnectionID = default(1);  
		
		//two methods exist to verifiy the interconnection ring state, link check and ring check
        //It is possible to enable both methods on manager, but it is not recommend
        //this parameter defines if an active layer 2 link check based on 802.1q-CFM is enabled on interconnection port
        //on manager it also enables the link status poll on start-up.
        bool linkCheckEnabled = default(true);
        
        //this parameter enables a periodic interconnection test frame sent out by interconnection manager
        //if this frame is not received within 5 interconnection test intervals, the interconnection ring is assumed broken
        //and interconnection manager set its interconnection port to FORWARDING
        bool ringCheckEnabled = default(false);	
		
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
        
        // The path to the InterfaceTable module
        string interfaceTableModule;
        
        // The path to the MacForwardingTable module
        string macTableModule;
        
        // The path to the MrpRelay module
        string mrpRelayModule;
        
        // Shows the ring by coloring connections in the network graphics.
        // Label ethernet interface with port role and status
        bool visualize = default(true);
        
        @signal[TestSignal](type="long");
        @signal[TopologyChangeSignal](type="long");
        @signal[LinkChangeSignal](type="long");
        @signal[ContinuityCheckSignal](type="long");
        @signal[ClearFDBSignal](type="long");
        @signal[ReceivedTestSignal](type="long");
        @signal[ReceivedChangeSignal](type="unsigned long");
        @signal[ReceivedContinuityCheckSignal](type="long");
        @signal[RingStateChangedSignal](type="long");
        @signal[PortStateChangedSignal](type="long");
        @statistic[TestFrameSignal](title="Testframes Sent"; source="TestSignal"; record=count,vector; interpolationmode=none);
        @statistic[LinkChangeSignal](title="LinkChangeSignal in us"; source="LinkChangeSignal"; record=count,vector; interpolationmode=none);
        @statistic[TopologyChangeSignal](title="TopologyChangeSignal in us"; source="TopologyChangeSignal"; record=count,vector; interpolationmode=none);
        @statistic[FDBCleared](title="FDB cleared in us"; source="ClearFDBSignal"; record=count,vector; interpolationmode=none);
        @statistic[RingTime](title="T Ring in us"; source="ReceivedTestSignal"; record=vector,stats,histogram; interpolationmode=none);
        @statistic[TimeToChange](title="Time till announced Change in ms"; source="ReceivedChangeSignal"; record=vector,stats,histogram; interpolationmode=none);
        @statistic[SentContinuityCheck](title="Sent Continuity Checks"; source="ContinuityCheckSignal"; record=count,vector; interpolationmode=none);
        @statistic[ReceivedContinuityCheck](title="Received Continuity Checks"; source="ReceivedContinuityCheckSignal"; record=count,vector; interpolationmode=none);
        @statistic[PortStateChanged](title="Time of Port State Change in us"; source="PortStateChangedSignal"; record=count,vector; interpolationmode=none);
        @statistic[RingStateChanged](title="Time of Ring State Change in us"; source="RingStateChangedSignal"; record=count,vector; interpolationmode=none);
        
        @signal[InTopologyChangeSignal](type="long");
        @statistic[InTopologyChangeSent](title="InterconnectionTopologyChange sent"; source="InTopologyChangeSignal"; record=count,vector; interpolationmode=none);
        
        @signal[InLinkChangeSignal](type="long");
        @statistic[InLinkChangeSignal](title="InterconnectionLinkChange sent"; source="InLinkChangeSignal"; record=count,vector; interpolationmode=none);
        
        @signal[ReceivedInTestSignal](type="long");
        @statistic[InterconnectionRingTime](title="T InterconnectionRing in us"; source="ReceivedInTestSignal"; record=count,vector; interpolationmode=none);
        
        @signal[ReceivedInChangeSignal](type="unsigned long");
        @statistic[TimeToInterconnectionChange](title="Time till announced InterconnectionChange"; source="ReceivedInChangeSignal"; record=vector,stats,histogram; interpolationmode=none);
        
        @signal[InStatusPollSignal](type="long");
        @statistic[InStatusPollSent](title="Interconnection Status Poll Sent in us"; source="InStatusPollSignal"; record=count,vector; interpolationmode=none);
        @signal[ReceivedInStatusPollSignal](type="long");
        @statistic[ReceivedInStatusPoll](title="Interconnection Status Poll received in us"; source="ReceivedInStatusPollSignal"; record=count,vector; interpolationmode=none);
        
        @signal[InTestSignal](type="long");
        @statistic[InTestFramesSent](title="Interconnection Test Frames Sent in us"; source="InTestSignal"; record=count,vector; interpolationmode=none);
        
        @signal[InterconnectionStateChangedSignal](type="long");
        @statistic[InterconnectionStateChanged](title="Time of Interconnection State Change"; source="InterconnectionStateChangedSignal"; record=count,vector; interpolationmode=none); 
        
    gates:
        input relayIn;
        output relayOut;
}


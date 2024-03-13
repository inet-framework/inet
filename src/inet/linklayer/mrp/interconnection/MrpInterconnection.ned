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

package inet.linklayer.mrp.interconnection;

import inet.linklayer.contract.IMediaRedundancy;
import inet.linklayer.mrp.mediaredundancy.Mrp;


simple MrpInterconnection extends Mrp like IMediaRedundancy
{
    parameters:
        @class(MrpInterconnection);        
        
        // each interconnection node has to take over one of two possible roles regarding interconection: 
        // interconnection client or interconnection manager
        // only one of the needed four nodes may have manager role (value 2), the other three nodes shall be in client role (value 1)
        int interconnectionRole = default(1);
 
        //PortNumber of interconnectionPort
        int interconnectionPort = default(2);       
             
        //each interconnection has an name assigned, for human readable identification
        string InterconnectionName = default("interconnection1");
        
        //each interconnection has an unique indentifier, assign a value from 0 - 65535
        int interconnectionID = default(1);  
        
        //two methods exist to verifiy the interconnection ring state, link check and ring check
        //It is possible to enable both methods on manager, but it is not recommend
        //this parameter defines if an active layer 2 link check based on 802.1q-CFM is enabled on interconnection port
        //on manager it also enables the link status poll on start-up.
        bool linkCheckEnabled = default(true);
        
        //this parameter enables a periodic interconnection test frame sent out by interconnection manager
        //if this frame is not received within 5 interconnection test intervals, the interconnection ring is assumed broken
        //and interconnection manager set its interconnection port to FORWARDING
        bool ringCheckEnabled = default(false);
        
     
        @display("i=block/network2");        
        @signal[InTopologyChangeSignal](type="long");
        @signal[InLinkChangeSignal](type="long");
        @signal[ReceivedInTestSignal](type="long");
        @signal[ReceivedInChangeSignal](type="unsigned long");
        @signal[InStatusPollSignal](type="long");
        @signal[ReceivedInStatusPollSignal](type="long");
        @signal[InTestSignal](type="long");
        @signal[InterconnectionStateChangedSignal](type="long");
        @signal[TestSignal](type="long");
        @signal[TopologyChangeSignal](type="unsigned long");
        @signal[LinkChangeSignal](type="double");
        @signal[ContinuityCheckSignal](type="long");
        @signal[ClearFDBSignal](type="double");
        @signal[ReceivedTestSignal](type="unsigned long");
        @signal[ReceivedChangeSignal](type="unsigned long");
        @signal[ReceivedContinuityCheckSignal](type="long");
        @signal[RingStateChangedSignal](type="unsigned long");
        @signal[PortStateChangedSignal](type="unsigned long");
        @statistic[TestFrameSignal](title="Testframes sent"; source="TestSignal"; record=count,vector; interpolationmode=none);
        @statistic[LinkChangeSignal](title="LinkChangeSignal in us"; source="LinkChangeSignal"; record=count,vector; interpolationmode=none);
        @statistic[TopologyChangeSignal](title="TopologyChangeSignal in us"; source="TopologyChangeSignal"; record=count,vector; interpolationmode=none);
        @statistic[FDBCleared](title="FDB cleared in us"; source="ClearFDBSignal"; record=count,vector; interpolationmode=none);
        @statistic[RingTime](title="T Ring in us"; source="ReceivedTestSignal"; record=vector,stats,histogram; interpolationmode=none);
        @statistic[TimeToChange](title="Time till announced Change in ms"; source="ReceivedChangeSignal"; record=vector,stats,histogram; interpolationmode=none);
        @statistic[SentContinuityCheck](title="Sent Continuity Checks"; source="ContinuityCheckSignal"; record=count,vector; interpolationmode=none);
        @statistic[ReceivedContinuityCheck](title="Received Continuity Checks"; source="ReceivedContinuityCheckSignal"; record=count,vector; interpolationmode=none);
        @statistic[PortStateChanged](title="Time of Port State Change in us"; source="PortStateChangedSignal"; record=count,vector; interpolationmode=none);
        @statistic[RingStateChanged](title="Time of Ring State Change in us"; source="RingStateChangedSignal"; record=count,vector; interpolationmode=none);
        @statistic[InTopologyChangeSent](title="InterconnectionTopologyChange sent"; source="InTopologyChangeSignal"; record=count,vector; interpolationmode=none);
        @statistic[InLinkChangeSignal](title="InterconnectionLinkChange sent"; source="InLinkChangeSignal"; record=count,vector; interpolationmode=none);
        @statistic[InterconnectionRingTime](title="T InterconnectionRing in us"; source="ReceivedInTestSignal"; record=count,vector; interpolationmode=none);
        @statistic[TimeToInterconnectionChange](title="Time till announced InterconnectionChange"; source="ReceivedInChangeSignal"; record=vector,stats,histogram; interpolationmode=none);
        @statistic[InStatusPollSent](title="Interconnection Status Poll Sent in us"; source="InStatusPollSignal"; record=count,vector; interpolationmode=none);
        @statistic[ReceivedInStatusPoll](title="Interconnection Status Poll received in us"; source="ReceivedInStatusPollSignal"; record=count,vector; interpolationmode=none);        
        @statistic[InTestFramesSent](title="Interconnection Test Frames Sent in us"; source="InTestSignal"; record=count,vector; interpolationmode=none);       
        @statistic[InterconnectionStateChanged](title="Time of Interconnection State Change"; source="InterconnectionStateChangedSignal"; record=count,vector; interpolationmode=none);            
}


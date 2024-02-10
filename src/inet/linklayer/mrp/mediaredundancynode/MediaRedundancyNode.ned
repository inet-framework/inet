//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.linklayer.mrp.mediaredundancynode;

import inet.linklayer.contract.IMediaRedundancy;

simple MediaRedundancyNode like IMediaRedundancy
{
    parameters:
        //time needed for linkdetection (up or down) in ms
        object linkDetectionDelayMean = default(383.10);
        object linkDetectionDelayDev = default(309.6);
        //processing delay in us
        object processingDelayMean = default(1000);
        object processingDelayDev = default(400);
        // Interval for Linkcheck: 3.3 ms or 10 ms
        double ccmInterval @unit(ms) = default(3.3ms);
        // Maximum Recovery Time for Ring. Can be 500 ms , 200 ms, 30 ms or 10 ms
        int timingProfile @unit(ms) = default(500ms);
        int expectedRoleByNum = default(0);       
        //Ringinterfaces by ID
        int ringPort1 = default(0);
		int ringPort2 = default(1);
		//Ring UUID, in 2 64-Bit parts
		int uuid0 = default(33333);
		int uuid1 = default(33333);	
		//LC
		bool interconnectionRingCheckAware = default(true);
        bool interconnectionLinkCheckAware = default(true);
		bool enableLinkCheckOnRing = default(false);			
		//manager variables
		bool nonBlockingMRC = default(true);
        bool reactOnLinkChange = default(true);
        bool checkMediaRedundancy = default(true);
        bool noTopologyChange = default(false);
        //client variables
        bool blockedStateSupported = default(true);
        // The path to the InterfaceTable module
        string interfaceTableModule;
        // The path to the MacForwardingTable module
        string macTableModule;
        string mrpRelayModule;
        // Shows the ring by coloring connections in the network graphics.
        // Label ethernet interface with port role and status
        bool visualize = default(true);
        @display("i=block/network2");
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
    gates:
        input relayIn;
        output relayOut;
}

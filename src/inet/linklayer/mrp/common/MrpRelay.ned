// Copyright (C) 2024 Daniel Zeitler
// SPDX-License-Identifier: LGPL-3.0-or-later

//
// This modul forwards frames (~EtherFrame) based on their destination MAC addresses to appropriate ports.
//
// It handles frame forwarding in accordance to interface data from both STP and MRP, and blocks traffic if 
// ports are set to BLOCKED.
// If a STP or MRP specific frame is received this frame is handled accordingly by forwarding based on the applied 
// rules in MrpMacForwardingTable and sending to upper layer mrp or stp node.
// @see EtherFrame, MrpMacForwardingTable, Stp, Rstp, MediaRedundancyNode, InterconnectionNode
//

package inet.linklayer.mrp.common;

import inet.linklayer.base.MacRelayUnitBase;
import inet.linklayer.contract.IMacRelayUnit;
import inet.linklayer.ieee8021d.relay.Ieee8021dRelay;

simple MrpRelay extends Ieee8021dRelay like IMacRelayUnit
{
    parameters:
        @class(MrpRelay);
        
        //switches take a certain amount of time for processing each packet.
        //this parameter sets the mean processsing delay and deviation in microseconds
        //the delay is applied beofre sending packets out.
        object switchingDelayMean = default(8.61);
    	object switchingDelayDev = default(5.42);
}


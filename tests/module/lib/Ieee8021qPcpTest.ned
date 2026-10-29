//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.test.moduletest.lib;

import inet.linklayer.ieee8021q.Ieee8021qTagEpdHeaderInserter;
import inet.linklayer.ieee8021q.Ieee8021qTagTpidHeaderInserter;
import inet.queueing.base.PacketFlowBase;
import inet.queueing.sink.PassivePacketSink;
import inet.queueing.source.ActivePacketSource;

//
// Test-only packet flow that attaches a C-tag VLAN request (10), optionally an
// S-tag VLAN request (100), and the user priority and PCP requests that its
// array parameters list for each packet, for testing the 802.1Q tag inserters.
//
simple PcpTestRequester extends PacketFlowBase
{
    parameters:
        object userPriorities = default([]); // User priority request of each packet; -1: none
        object cPcps = default([]); // C-tag PCP request of each packet; -1: none
        object sPcps = default([]); // S-tag PCP request of each packet; -1: none
        bool sTag = default(false);
        bool epd = default(true);
        @class(::inet::PcpTestRequester);
}

//
// Test-only packet flow that prints the PCP of each 802.1Q tag at the front of the
// packet, and its user priority and PCP requests and indications, to stdout.
//
simple PcpTestProbe extends PacketFlowBase
{
    parameters:
        int numHeaders = default(1);
        bool tpid = default(false);
        @class(::inet::PcpTestProbe);
}

//
// A source, a PcpTestRequester, an EPD C-tag inserter, optionally an EPD S-tag
// inserter, a PcpTestProbe and a sink.
//
module PcpTestChain
{
    parameters:
        bool sTag = default(false);
    submodules:
        source: ActivePacketSource {
            productionInterval = 1s;
            packetLength = 46B;
        }
        requester: PcpTestRequester {
            sTag = parent.sTag;
        }
        cInserter: Ieee8021qTagEpdHeaderInserter {
            vlanTagType = "c";
        }
        sInserter: Ieee8021qTagEpdHeaderInserter if sTag {
            vlanTagType = "s";
        }
        probe: PcpTestProbe {
            numHeaders = parent.sTag ? 2 : 1;
        }
        sink: PassivePacketSink;
    connections:
        source.out --> requester.in;
        requester.out --> cInserter.in;
        cInserter.out --> sInserter.in if sTag;
        sInserter.out --> probe.in if sTag;
        cInserter.out --> probe.in if !sTag;
        probe.out --> sink.in;
}

//
// A source, a PcpTestRequester, a TPID C-tag inserter, a PcpTestProbe and a sink.
//
module PcpTestTpidChain
{
    submodules:
        source: ActivePacketSource {
            productionInterval = 1s;
            packetLength = 46B;
        }
        requester: PcpTestRequester {
            epd = false;
        }
        cInserter: Ieee8021qTagTpidHeaderInserter {
            vlanTagType = "c";
        }
        probe: PcpTestProbe {
            tpid = true;
        }
        sink: PassivePacketSink;
    connections:
        source.out --> requester.in;
        requester.out --> cInserter.in;
        cInserter.out --> probe.in;
        probe.out --> sink.in;
}

//
// Copyright (C) 2008 Juan-Carlos Maureira
// Copyright (C) INRIA
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.dhcp;

import inet.common.SimpleModule;
import inet.applications.contract.IApp;

//
// Implements the DHCP client protocol. DHCP (Dynamic Host Configuration Protocol),
// described in RFC 2131, provides configuration parameters to Internet hosts.
// Requires UDP.
//
// @see ~DhcpServer, ~DhcpMessage
//
simple DhcpClient extends SimpleModule like IApp
{
    parameters:
        @class(DhcpClient);
        string interfaceTableModule; // Path to the InterfaceTable module
        string routingTableModule; // Path to the Ipv4RoutingTable module
        string interface = default(""); // Which NIC should be configured (i.e. wlan0, eth1 etc.)
        double startTime @unit(s) = default(0s); // Starting time of DHCP initialization
        // Retransmission strategy for DHCPDISCOVER/DHCPREQUEST while waiting for OFFER/ACK
        // in INIT, SELECTING, REQUESTING and REBOOTING states (RFC 2131 §4.1).
        double initialRetransmitDelay @unit(s) = default(4s); // First retransmit wait (jittered by +-1s)
        double maxRetransmitDelay @unit(s) = default(64s); // Per-attempt wait is doubled up to this cap
        int maxRetransmitCount = default(4); // After this many retransmits with no reply, give up and restart from INIT
        // Retransmission strategy in RENEWING/REBINDING (RFC 2131 §4.4.5):
        // half of the remaining interval to T2 (RENEWING) or to lease expiry (REBINDING),
        // floored at this minimum.
        double minRenewRetransmitInterval @unit(s) = default(60s);
        // Test hook for DHCPDECLINE: if non-empty, the next DHCPACK granting
        // an IP equal to this address triggers a DHCPDECLINE (RFC 2131 §4.3.3)
        // instead of a normal bind. After declining, the address is cleared
        // so subsequent ACKs are accepted normally. A real client would set
        // this from an ARP-probe-detected conflict (RFC 5227); the model
        // exposes the hook directly because INET's Arp module does not yet
        // surface the information needed to do passive probe-reply detection.
        string declineOfferedIp = default("");
        displayStringTextFormat = default("{clientStateName}");
        @display("i=block/app2");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // Extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // Timeout value for lifecycle stop operation
        @selfMessageKinds(inet::DhcpTimerType);
    gates:
        input socketIn @labels(UdpCommand/up);
        output socketOut @labels(UdpCommand/down);
}


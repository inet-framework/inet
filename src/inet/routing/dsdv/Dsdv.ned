// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.routing.dsdv;

import inet.applications.contract.IApp;

// Perkins/Bhagwat (SIGCOMM 1994), network-layer host-routing profile.
// One bidirectional ad hoc IPv4 interface; finite-run sequence numbers.
// This versioned UDP model profile is not an external interoperability format.
simple Dsdv like IApp
{
    parameters:
        @class(dsdv::Dsdv);
        @display("i=block/network2");
        @lifecycleSupport;
        string interfaceTableModule = default(absPath("^.interfaceTable"));
        string routingTableModule = default(absPath("^.ipv4.routingTable"));
        string interface = default("wlan0"); // Exactly one active non-loopback interface is supported
        int controlPort = default(26999); // Unprivileged UDP port, 1024..65535
        double fullDumpInterval @unit(s) = default(5s); // Positive periodic interval
        double periodicJitter @unit(s) = default(500ms); // Uniform [0,J], nonnegative
        double neighborTimeout @unit(s) = default(20s); // Positive sender liveness timeout
        double routeTimeout @unit(s) = default(25s); // Positive per-destination freshness timeout
        double settlingCap @unit(s) = default(1s); // Positive maximum finite-change delay
        int payloadBudget @unit(B) = default(1400B); // UDP payload; also limited by interface MTU minus 28B
        @signal[updateAccepted](type=long);
        @signal[updateRejected](type=long);
        @signal[routeInstalled](type=long);
        @signal[routeWithdrawn](type=long);
        @signal[invalidationReason](type=string);
        @signal[controlBytes](type=long);
        @signal[controlPacketSent](type=long);
        @signal[triggeredSend](type=long);
        @signal[fullDumpSent](type=long);
        @signal[incrementalSent](type=long);
        @signal[settlingDelay](type=simtime_t);
        @statistic[updateAccepted](record=count);
        @statistic[updateRejected](record=count);
        @statistic[routeInstalled](record=count);
        @statistic[routeWithdrawn](record=count);
        @statistic[controlBytes](record=sum);
        @statistic[controlPacketSent](record=count);
        @statistic[fullDumpSent](record=count);
        @statistic[incrementalSent](record=count);
        @statistic[settlingDelay](record=vector);
    gates:
        input socketIn;
        output socketOut;
}

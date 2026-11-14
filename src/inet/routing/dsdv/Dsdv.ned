// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.routing.dsdv;

import inet.applications.contract.IApp;

// Perkins/Bhagwat (SIGCOMM 1994), network-layer host-routing profile.
// One bidirectional ad hoc IPv4 interface; an optional base station has a wired uplink.
// This versioned UDP model profile is not an external interoperability format.
simple Dsdv like IApp
{
    parameters:
        @class(dsdv::Dsdv);
        @display("i=block/network2");
        @lifecycleSupport;
        string interfaceTableModule = default(absPath("^.interfaceTable"));
        string routingTableModule = default(absPath("^.ipv4.routingTable"));
        string arpModule = default(absPath("^.ipv4.arp")); // Used only with MAC failure feedback
        string interface = default("wlan0"); // Ad hoc interface; only a configured base station may add externalInterface
        bool useMacFailureFeedback = default(false); // Use unicast MAC delivery failures as neighbor-loss hints
        string baseStationAddress = default(""); // Empty selects host-only v1; otherwise every participant uses v2 and this fixed IPv4 origin
        string externalInterface = default(""); // Optional extra uplink on the matching base station only
        int controlPort = default(26999); // Unprivileged UDP port, 1024..65535
        double fullDumpInterval @unit(s) = default(5s); // Positive periodic interval
        double periodicJitter @unit(s) = default(500ms); // Uniform [0,J], nonnegative
        bool adaptiveFullDump = default(false); // Adapt periodic intervals to selected-route changes and payload overhead
        double minFullDumpInterval @unit(s) = default(fullDumpInterval); // Positive lower bound, at most the initial interval
        double maxFullDumpInterval @unit(s) = default(2 * fullDumpInterval); // Upper bound, at least the initial interval
        double fullDumpBitrateBudget @unit(bps) = default(11200bps); // Positive periodic DSDV payload target, not total channel load
        double neighborTimeout @unit(s) = default(20s); // Positive sender liveness timeout
        double routeTimeout @unit(s) = default(25s); // Positive per-destination freshness timeout
        double settlingCap @unit(s) = default(1s); // Positive maximum finite-change delay
        double minSettlingDelay @unit(s) = default(10ms); // Positive minimum deferred-change delay, at most settlingCap
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
        @signal[fullDumpIntervalSelected](type=simtime_t);
        @statistic[updateAccepted](record=count);
        @statistic[updateRejected](record=count);
        @statistic[routeInstalled](record=count);
        @statistic[routeWithdrawn](record=count);
        @statistic[controlBytes](record=sum);
        @statistic[controlPacketSent](record=count);
        @statistic[fullDumpSent](record=count);
        @statistic[incrementalSent](record=count);
        @statistic[settlingDelay](record=vector);
        @statistic[fullDumpIntervalSelected](record=vector; unit=s);
    gates:
        input socketIn;
        output socketOut;
}

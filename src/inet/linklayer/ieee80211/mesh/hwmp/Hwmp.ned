// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.linklayer.ieee80211.mesh.hwmp;
import inet.common.SimpleModule;
import inet.linklayer.ieee80211.mesh.contract.IMeshPathSelection;

// Native on-demand HWMP. Root procedures and path-error signaling are separate.
simple Hwmp extends SimpleModule like IMeshPathSelection
{
    parameters:
        @class(Hwmp);
        @lifecycleSupport;
        string mibModule;
        string managementModule = default("^.mgmt");
        string metricModule = default("^.linkMetric");
        string forwardingModule = default("^.mac.ds");
        double preqMinInterval @unit(s) = default(102.4ms); // dot11MeshHWMPpreqMinInterval,100TU
        double netDiameterTraversalTime @unit(s) = default(512ms); //500TU
        double activePathTimeout @unit(s) = default(5.12s); //5000TU
        int maxPreqRetries = default(3); // dot11MeshHWMPmaxPREQretries
        int initialTtl = default(31);
        int seenIdentityCapacity = default(4096); // model bound; retain identities for received Lifetime
        @signal[hwmpPathUpdated](type=bool); // current data usability, details: Hwmp::PathChange
        @signal[hwmpDiscoveryChanged](type=bool); // active queue-driven attempt, details: Hwmp::DiscoveryChange
        @display("i=block/routing");
}

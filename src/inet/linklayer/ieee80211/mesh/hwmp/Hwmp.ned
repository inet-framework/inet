// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.linklayer.ieee80211.mesh.hwmp;
import inet.common.SimpleModule;
import inet.linklayer.ieee80211.mesh.contract.IMeshPathSelection;

// Native reactive and proactive HWMP with path-error maintenance.
simple Hwmp extends SimpleModule like IMeshPathSelection
{
    parameters:
        @class(Hwmp);
        @lifecycleSupport;
        string mibModule;
        string managementModule = default("^.mgmt");
        string metricModule = default("^.linkMetric");
        string forwardingModule = default("^.mac.ds");
        string hcfModule = default("^.mac.hcf");
        double preqMinInterval @unit(s) = default(102.4ms); // dot11MeshHWMPpreqMinInterval,100TU
        double perrMinInterval @unit(s) = default(102.4ms); // dot11MeshHWMPperrMinInterval,100TU
        double netDiameterTraversalTime @unit(s) = default(512ms); //500TU
        double activePathTimeout @unit(s) = default(5.12s); //5000TU
        int maxPreqRetries = default(3); // dot11MeshHWMPmaxPREQretries
        int initialTtl = default(31);
        int seenIdentityCapacity = default(4096); // model bound; retain identities for received Lifetime
        int rootMode = default(0); //0 disabled,2 proactive PREQ flag0,3 proactive PREQ flag1,4 RANN
        double rootInterval @unit(s) = default(2.048s); //2000TU
        double rannInterval @unit(s) = default(2.048s); //initial shared MIB value,2000TU
        double confirmationInterval @unit(s) = default(2.048s); //2000TU
        double activePathToRootTimeout @unit(s) = default(5.12s); //5000TU
        @signal[hwmpPathUpdated](type=bool); // current data usability, details: Hwmp::PathChange
        @signal[hwmpDiscoveryChanged](type=bool); // active queue-driven attempt, details: Hwmp::DiscoveryChange
        @signal[hwmpRootUpdated](type=bool); // current root path usability, copied details: Hwmp::RootChange
        @display("i=block/routing");
}

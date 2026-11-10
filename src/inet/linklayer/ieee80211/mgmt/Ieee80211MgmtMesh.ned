// SPDX-License-Identifier: LGPL-3.0-or-later

package inet.linklayer.ieee80211.mgmt;

import inet.common.SimpleModule;

// IEEE Std 802.11-2024 open mesh peering and neighbor-offset synchronization.
// Always awake OFDM profile; this component does not provide mesh forwarding.
simple Ieee80211MgmtMesh extends SimpleModule like IIeee80211Mgmt
{
    parameters:
        @class(Ieee80211MgmtMesh);
        string interfaceTableModule;
        string mibModule;
        string macModule;
        string linkMetricModule = default("^.linkMetric");
        string meshId = default("inet-mesh");
        int pathSelectionMetricId = default(1); // mesh-wide fixed ID, 1 or 2
        double beaconInterval @unit(s) = default(102.4ms); // whole TUs, 1..65535
        int dtimPeriod = default(1); // 1..255 beacon periods
        double meshAwakeWindow @unit(s) = default(10.24ms); // whole TUs, advertised on DTIM
        double initialTsf @unit(s) = default(0s); // model policy: initial independent TSF value
        double retryTimeout @unit(s) = default(40ms); // dot11MeshRetryTimeout, Annex C
        double confirmTimeout @unit(s) = default(40ms); // dot11MeshConfirmTimeout, Annex C
        double holdingTimeout @unit(s) = default(40ms); // dot11MeshHoldingTimeout, Annex C
        int maxRetries = default(2); // dot11MeshMaxRetries, 0..16
        int maxPeerings = default(32); // bounded local admission policy, 1..2007
        bool autoPeer = default(true); // local SME policy: initiate on a compatible beacon/probe response
        @signal[meshPeeringStateChanged](type=long); // value: PeerState; details: MeshPeerChange
        @signal[meshManagementDiscarded](type=long); // one rejected/unsupported management packet
        @signal[meshBeaconTransmissionEnd](type=double); // actual PPDU completion time, seconds
        @display("i=block/cogwheel");
    gates:
        input macIn;
        output macOut;
        input agentIn @loose;
        output agentOut @loose;
}

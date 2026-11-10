// SPDX-License-Identifier: LGPL-3.0-or-later

package inet.linklayer.ieee80211;

import inet.linklayer.ieee80211.mesh.contract.IMeshPathSelection;

// Open, single-channel QoS mesh with native OFDM forwarding. Individual traffic
// uses an optional path provider; HT/VHT, aggregation and power save are separate.
module Ieee80211MeshInterface extends Ieee80211Interface
{
    parameters:
        string meshId = default("inet-mesh");
        int pathSelectionMetricId = default(1); // 1=airtime, 2=high-PHY-rate airtime; fixed for the mesh BSS
        opMode = default("a");
        bitrate = default(6Mbps);
        agent.typename = "";
        mgmt.typename = default("Ieee80211MgmtMesh");
        mac.typename = default("Ieee80211MeshMac");
        mac.ds.typename = default("MeshForwarding");
        radio.bandName = default("5 GHz (20 MHz)");
        radio.bandwidth = default(20MHz);
        radio.receiver.bandwidth = default(20MHz);
        mgmt.meshId = this.meshId;
        mgmt.pathSelectionMetricId = this.pathSelectionMetricId;
        mac.qosStation = true;
        mac.hcf.isBlockAckSupported = false;
        mac.hcf.originatorMacDataService.msduAggregationPolicy.typename = "";
        mac.hcf.originatorMacDataService.mpduAggregationPolicy.typename = "";
        mac.hcf.originatorMacDataService.fragmentationPolicy.typename = "";
        mac.hcf.rtsPolicy.rtsThreshold = default(65536B);
    submodules:
        pathSelection: <default("")> like IMeshPathSelection if typename != "" {
            parameters:
                @display("p=100,200");
        }
}

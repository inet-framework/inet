// SPDX-License-Identifier: LGPL-3.0-or-later

package inet.linklayer.ieee80211;

import inet.linklayer.ieee80211.mesh.contract.IMeshPathSelection;
import inet.linklayer.ieee80211.mesh.AirtimeLinkMetric;
import inet.linklayer.ieee80211.mesh.contract.IMeshPowerSave;

// Open QoS mesh with native forwarding and optional contiguous5GHz HT/VHT PHY.
// Native source A-MSDU is available through the MSDU aggregation policy.
// Immediate Compressed Block Ack enables optional native A-MPDU; power save
// remains outside the always-awake profile.
module Ieee80211MeshInterface extends Ieee80211Interface
{
    parameters:
        string meshId = default("inet-mesh");
        bool powerSave = default(false); // Activation requires the complete service-period engine.
        string phyProfile @enum("ofdm","ht","vht") = default("ofdm");
        int pathSelectionMetricId = default(phyProfile == "ofdm" ? 1 : 2); // profile choice, independent of negotiated instantaneous rate
        opMode = default(phyProfile == "ofdm" ? "a" : "ac-complete");
        bitrate = default(phyProfile == "ofdm" ? 6Mbps : -1bps);
        agent.typename = "";
        mgmt.typename = default("Ieee80211MgmtMesh");
        mac.typename = default("Ieee80211MeshMac");
        mac.ds.typename = default("MeshForwarding");
        radio.bandName = default(phyProfile == "ofdm" ? "5 GHz (20 MHz)" : "5 GHz (mapped 20 MHz)");
        radio.contiguousChannelAllocation = default(phyProfile != "ofdm");
        radio.bandwidth = default(20MHz);
        radio.receiver.bandwidth = default(20MHz);
        mgmt.meshId = this.meshId;
        mac.phyProfile = this.phyProfile;
        mgmt.pathSelectionMetricId = this.pathSelectionMetricId;
        mac.qosStation = true;
        mac.hcf.isBlockAckSupported = default(false);
        mac.hcf.meshLinkMetricModule = "^.^.linkMetric";
        mac.hcf.meshPowerSaveModule = "^.^.powerSavePolicy";
        mac.hcf.edca.edcaf[*].pendingQueue.packetCapacity = default(powerSave ? 64 : 100);
        mac.hcf.edca.edcaf[*].pendingQueue.dataCapacity = default(powerSave ? 256KiB : -1b);
        mac.hcf.originatorMacDataService.msduAggregationPolicy.typename = default("");
        mac.hcf.originatorMacDataService.mpduAggregationPolicy.typename = default("");
        mac.hcf.originatorMacDataService.fragmentationPolicy.typename = "";
        mac.hcf.rtsPolicy.rtsThreshold = default(65536B);
    submodules:
        powerSavePolicy: <default("MeshPowerSave")> like IMeshPowerSave {
            parameters:
                enabled = parent.powerSave;
                @display("p=100,300");
        }
        linkMetric: AirtimeLinkMetric {
            parameters:
                @display("p=100,100");
        }
        pathSelection: <default("Hwmp")> like IMeshPathSelection if typename != "" {
            parameters:
                @display("p=100,200");
        }
}

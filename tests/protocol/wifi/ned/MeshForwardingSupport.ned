// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.protocoltest.wifi;
import inet.linklayer.ieee80211.mesh.contract.IMeshPathSelection;
import inet.linklayer.ieee80211.llc.IIeee80211Llc;

simple MeshFixturePaths like IMeshPathSelection {
    parameters: @class(::MeshFixturePaths);
}
simple MeshFixtureLlc like IIeee80211Llc {
    parameters: @class(::MeshFixtureLlc);
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}

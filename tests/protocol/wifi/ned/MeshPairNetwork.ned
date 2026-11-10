// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.protocoltest.wifi;

import inet.node.inet.WirelessHost;
import inet.physicallayer.wireless.common.contract.packetlevel.IRadioMedium;
import inet.protocoltest.ProtocolTester;

network MeshPairNetwork
{
    parameters:
        int numStations = default(2);
    submodules:
        radioMedium: <default("Ieee80211ScalarRadioMedium")> like IRadioMedium;
        tester: ProtocolTester;
        station[numStations]: WirelessHost;
}

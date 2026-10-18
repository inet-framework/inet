// SPDX-License-Identifier: LGPL-3.0-or-later

package inet.examples.wireless.htampdu;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.wireless.common.contract.packetlevel.IRadioMedium;

network HtAmpdu
{
    submodules:
        configurator: Ipv4NetworkConfigurator;
        radioMedium: <default("Ieee80211ScalarRadioMedium")> like IRadioMedium;
        ap: AccessPoint;
        source: WirelessHost;
        destination: WirelessHost;
}

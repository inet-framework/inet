//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.wireless.pathloss;

import inet.environment.common.PhysicalEnvironment;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;


network PathLossShowcase
{
    @display("bgb=1400,400");
    @statistic[receptionPower](source="receptionMinSignalPower(radioMedium.signalArrivalStarted)"; record=last);
    submodules:
        source: AdhocHost {
            @display("p=144.37871,110.05918");
        }
        destination: AdhocHost {
            @display("p=226.62724,110.05918");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=49.704147,35.502964");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=49.704147,85.20711");
        }
        physicalEnvironment: PhysicalEnvironment {
            @display("p=49.704147,135.50298");
        }
}

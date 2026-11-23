// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.examples.dsdv;

import inet.node.dsdv.DsdvRouter;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.physicallayer.wireless.common.medium.UnitDiskRadioMedium;

// Four nodes separated by 100m. The second node leaves and returns in Mobile.
network DsdvMobileLine
{
    submodules:
        radioMedium: UnitDiskRadioMedium;
        configurator: Ipv4NetworkConfigurator;
        host[4]: DsdvRouter;
}

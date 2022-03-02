//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.showcases.tsn.gatescheduling;

import inet.node.tsn.TsnSwitch;

module TsnSwitch1 extends TsnSwitch
{
    @defaultStatistic("gateState:vector"; module="eth[2].macLayer.queue.transmissionGate[0]");
}

module TsnSwitch2 extends TsnSwitch
{
    @defaultStatistic("gateState:vector"; module="eth[0].macLayer.queue.transmissionGate[0]");
}

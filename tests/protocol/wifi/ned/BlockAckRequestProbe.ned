// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.protocoltest.wifi;

import inet.linklayer.ieee80211.mac.Ieee80211Mac;

module BlockAckRequestProbeHcf extends BlockAckLifecycleHcf {
    parameters:
        @class(::inet::protocoltest::BlockAckRequestProbeHcf);
}
module BlockAckRequestProbeMac extends Ieee80211Mac {
    parameters:
        @class(::inet::protocoltest::BlockAckRequestProbeMac);
        @signal[probeResponseAcknowledged](type=long);
        @signal[probeDelbaAcknowledged](type=long);
}
simple BlockAckRequestProbeStimulus {
    parameters:
        bool active = default(true);
        double timeout @unit(s) = default(0s);
        @class(::inet::protocoltest::BlockAckRequestProbeStimulus);
}
network BlockAckRequestProbeNetwork extends WifiInfraNetwork {
    submodules:
        stimulus: BlockAckRequestProbeStimulus;
}

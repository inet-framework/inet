// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.protocoltest.wifi;

import inet.linklayer.ieee80211.mac.coordinationfunction.Hcf;

module BlockAckLifecycleHcf extends Hcf
{
    parameters:
        @class(::inet::protocoltest::BlockAckLifecycleHcf);
}

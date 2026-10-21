// SPDX-License-Identifier: LGPL-3.0-or-later

package inet.protocoltest;

import inet.physicallayer.wireless.noise.NoiseSource;

// This source emits one noise transmission per protocol-test action.
// The inherited transmitter and mobility parameters define its physical properties.
module TriggeredNoiseSource extends NoiseSource
{
    parameters:
        sleepInterval = default(0s); // This source does not use periodic transmissions.
        @class(TriggeredNoiseSource);
        @signal[interferenceStarted](type=long);
        @signal[interferenceEnded](type=long);
}

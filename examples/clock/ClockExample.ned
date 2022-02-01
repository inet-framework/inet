//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.clock;

import inet.clock.model.IdealClock;
import inet.clock.model.OscillatorBasedClock;

network ClockExample
{
    parameters:
        int numClock;
    submodules:
        referenceClock: IdealClock {
            @display("p=200,100");
        }
        clock[numClock]: OscillatorBasedClock {
            @display("p=200,200,column,100");
        }
}

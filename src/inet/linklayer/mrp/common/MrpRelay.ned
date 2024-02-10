// Copyright (C) 2024 Daniel Zeitler
// SPDX-License-Identifier: LGPL-3.0-or-later

package inet.linklayer.mrp.common;

import inet.linklayer.base.MacRelayUnitBase;
import inet.linklayer.contract.IMacRelayUnit;
import inet.linklayer.ieee8021d.relay.Ieee8021dRelay;

simple MrpRelay extends Ieee8021dRelay like IMacRelayUnit
{
    parameters:
        @class(MrpRelay);
        //switching delay in us
        object switchingDelayMean = default(8.61);
    	object switchingDelayDev = default(5.42);
}


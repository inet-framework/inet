//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.common;

import inet.linklayer.base.MacRelayUnitBase;
import inet.linklayer.contract.IMacRelayUnit;

simple MacRelayUnit extends MacRelayUnitBase like IMacRelayUnit
{
    parameters:
        @class(MacRelayUnit);
}


//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.shaper;

//
// This module implements a credit based shaper using a gate submodule to manage
// the number of credits.
//
// @see ~CreditBasedGate
//
module CreditBasedShaper extends PacketShaper
{
    parameters:
        gate.typename = default("CreditBasedGate");
}

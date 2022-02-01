//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.ieee8022;

//
// This module implements an IEEE 802.2 LLC application that only receives packets.
//
module Ieee8022LlcSinkApp extends Ieee8022LlcApp
{
    parameters:
        source.typename = "";
        io.destPort = -1;
}

//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.ieee8022;

//
// This module implements an IEEE 802.2 LLC application that only sends packets.
//
module Ieee8022LlcSourceApp extends Ieee8022LlcApp
{
    parameters:
        sink.typename = "";
}

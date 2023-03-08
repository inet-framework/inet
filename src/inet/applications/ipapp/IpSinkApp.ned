//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.ipapp;

//
// This module implements a IP application that only receives packets.
//
module IpSinkApp extends IpApp
{
    parameters:
        source.typename = "";
}

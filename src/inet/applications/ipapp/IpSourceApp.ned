//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.ipapp;

//
// This module implements an IP application that only sends packets.
//
module IpSourceApp extends IpApp
{
    parameters:
        sink.typename = "";
}

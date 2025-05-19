//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.quicapp;

import inet.common.SimpleModule;
import inet.applications.contract.IApp;

simple QuicClient extends SimpleModule like IApp
{
    parameters:
        @class(QuicClient);
        string localAddress = default(""); // comma separated list of addresses
        int localPort = default(0); // port number to listen on
        string connectAddress;  // server address (may be symbolic)
        int connectPort; // port number to connect to
    gates:
        input socketIn;
        output socketOut;
}

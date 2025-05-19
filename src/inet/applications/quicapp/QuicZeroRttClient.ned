//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.quicapp;

import inet.common.SimpleModule;
import inet.applications.contract.IApp;

simple QuicZeroRttClient extends SimpleModule like IApp
{
    parameters:
        @class(QuicZeroRttClient);
        string localAddress = default(""); // comma separated list of addresses
        int localPort = default(0); // port number to listen on
        string connectAddress;  // server address (may be symbolic)
        int connectPort; // port number to connect to
        bool invalidClientTokenString = default(false); // app provides a client token with an invalid format (empty string)
        bool sendInvalidToken = default(false); // app provides a client token with a valid format but a wrong token number
    gates:
        input socketIn;
        output socketOut;
}

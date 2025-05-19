//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.quicapp;

import inet.common.SimpleModule;
import inet.applications.contract.IApp;

// A QUIC server that reads and discards all received data.
simple QuicOrderedReceiver extends SimpleModule like IApp
{
    parameters:
        @class(QuicOrderedReceiver);
        @signal[bytesReceived];
        @statistic[bytesReceived](record=vector);
        
        string localAddress = default(""); // comma separated list of addresses
        int localPort = default(0); // port number to listen on
    gates:
        input socketIn;
        output socketOut;
}

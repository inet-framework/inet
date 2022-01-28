//
// Copyright (C) 2008 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.applications.sctpapp;

import inet.applications.contract.IApp;

simple SctpNatServer like IApp
{
    parameters:
        string localAddress = default(""); // may be left empty ("")
        int localPort = default(0); // port number to listen on
        int numPacketsToSendPerClient = default(0);  // number of requests sent per session
        int numPacketsToReceivePerClient = default(1);
        int requestLength = default(1452); // length of a request (bytes)
        double waitToClose @unit(s) = default(0s); //time to wait between last message sent and abort
        int outboundStreams = default(1);
        int inboundStreams = default(1);
        bool ordered = default(true);
    gates:
        input socketIn;
        output socketOut;
}

